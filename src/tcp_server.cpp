#include "tcp_server.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cerrno>
#include <system_error>

namespace aether {
TcpServer::TcpServer(uint16_t port) : port_(port) {}

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
}  // namespace aether
