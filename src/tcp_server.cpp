#include "tcp_server.hpp"
#include "file_descriptor.hpp"
#include "thread_pool.hpp" 
#include "client.hpp"

#include <arpa/inet.h>
#include <csignal>
#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <cerrno>
#include <system_error>

namespace aether {
TcpServer::TcpServer(uint16_t port) : port_(port) {}

volatile std::sig_atomic_t g_stop = 0;

void handle_signal(int) {
    g_stop = 1;
}

void TcpServer::setup_signals() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
}

void TcpServer::listen_start() {
    // 1. Create socket (IPv4, TCP stream)
    int raw_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (raw_fd < 0) {
        throw std::system_error(errno, std::generic_category(), "Failed to create socket");
    }

    // Wrap raw file descriptor into RAII container
    listen_fd_ = FileDescriptor(raw_fd);

    // Allow port reuse to avoid "Address already use" errors during rapid restarts
    int opt = 1;
    if (::setsockopt(listen_fd_.get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::system_error(errno, std::generic_category(), "Failed to set SO_REUSEADDR");
    }

    // 3. Configure socket address structure and bind to the specified port
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);
    if (::bind(listen_fd_.get(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        throw std::system_error(errno, std::generic_category(), "Failed to bind socket");
    }

    // 4. Mark the socket as a passive server listening for incoming connections
    if (::listen(listen_fd_.get(), SOMAXCONN) < 0) {
        throw std::system_error(errno, std::generic_category(), "Failed to listen on socket");
    }
}

FileDescriptor TcpServer::accept_one(std::string& peer_ip, uint16_t& peer_port) {
    // Structure to hold client address info populated by the kernel
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    // 1. Accept an incoming connection and get a dedicated socket descriptor
    int client_fd =
        ::accept(listen_fd_.get(), reinterpret_cast<sockaddr*>(&client_addr), &client_len);
    if (client_fd < 0) {
        throw std::system_error(errno, std::generic_category(), "accept failed");
    }

    // 2. Convert binary IP address to human-readable string (e.g., "127.0.0.1")
    char ip_str[INET_ADDRSTRLEN];
    if (::inet_ntop(AF_INET, &(client_addr.sin_addr), ip_str, sizeof(ip_str)) != nullptr) {
        peer_ip = ip_str;
    } else {
        peer_ip = "unknown";
    }

    // 3. Convert port from Network Byte Order (Big-Endian) to Host Byte Order (Little-Endian)
    peer_port = ntohs(client_addr.sin_port);

    // 4. Wrap raw socket handle in RAII FileDescriptor for automatic resource management
    return FileDescriptor(client_fd);
}

void TcpServer::run() {
    listen_start();
    while (!g_stop) {
        struct pollfd pfd;
        pfd.fd = listen_fd_.get();
        pfd.events = POLLIN;
        pfd.revents = 0;

        int ready = poll(&pfd, 1, 500);

        if (ready < 0) {
            if (errno == EINTR) {
                continue; 
            }
            break;
        }
        if (ready == 0) {
            continue;
        }
        if (pfd.revents & POLLIN) {
            std::string client_ip;
            uint16_t client_port = 0;
            FileDescriptor client_fd = accept_one(client_ip, client_port);

            auto conn_ptr = std::make_shared<FileDescriptor>(std::move(client_fd));

            pool_.submit([conn_ptr]() {
                handle_echo(conn_ptr);
            });
        }
    }
}
}  // namespace aether
