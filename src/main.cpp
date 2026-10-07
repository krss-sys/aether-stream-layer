#include <sys/socket.h>

#include <atomic>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <memory>
#include <system_error>
#include <thread>

#include "logger.hpp"
#include "net_io.hpp"
#include "tcp_server.hpp"

// Atomic global counter to safely track active connections across multiple threads
std::atomic<int> g_active_connections{0};

// RAII guard to automatically manage the active connection counter
struct ConnectionGuard {
    ConnectionGuard() {
        ++g_active_connections;
        aether::Logger::log(
            aether::Logger::Level::Info,
            "Client connected (active: " + std::to_string(g_active_connections) + ")");
    }

    ~ConnectionGuard() {
        --g_active_connections;
        aether::Logger::log(
            aether::Logger::Level::Info,
            "Client disconnected (active: " + std::to_string(g_active_connections) + ")");
    }
};

void handle_echo(aether::FileDescriptor client_fd) {
    // Automatically increment counter on entry and decrement on exit
    ConnectionGuard guard;
    char buffer[1024];

    while (true) {
        // Receive data safely, handling EINTR retries
        ssize_t bytes_received = aether::recv_some(client_fd.get(), buffer, sizeof(buffer));

        if (bytes_received > 0) {
            // Send all data back, handling partial writes, EINTR, and preventing SIGPIPE
            bool ok =
                aether::send_all(client_fd.get(), buffer, static_cast<size_t>(bytes_received));
            if (!ok) {
                aether::Logger::log(aether::Logger::Level::Error,
                                    "Send error, closing connection!");
                break;
            }
        } else if (bytes_received == 0) {
            // Peer closed connection gracefully
            break;
        } else {
            // Error occurred during receive
            std::string err_msg = "Recv error: ";
            err_msg += std::strerror(errno);
            aether::Logger::log(aether::Logger::Level::Error, err_msg);
            break;
        }
    }
}

int main() {
    try {
        // Initialize TCP server bound to port 8080
        aether::TcpServer server(8080);

        // Start listening for incoming connections
        server.listen_start();
        aether::Logger::log(aether::Logger::Level::Info, "Server is listening on port 8080...");

        while (true) {
            std::string client_ip;
            uint16_t client_port = 0;

            // Block and wait to accept a single client connection
            aether::FileDescriptor client_fd = server.accept_one(client_ip, client_port);

            std::string conn_msg =
                "Accepted connection from " + client_ip + ":" + std::to_string(client_port);
            aether::Logger::log(aether::Logger::Level::Info, conn_msg);

            auto conn = std::make_shared<aether::FileDescriptor>(std::move(client_fd));

            std::thread([c = std::move(conn)]() { handle_echo(std::move(*c)); }).detach();
        }
    } catch (const std::system_error& e) {
        std::string err_msg = "System error: ";
        err_msg += e.what();
        err_msg += " (code: " + std::to_string(e.code().value()) + ")";
        aether::Logger::log(aether::Logger::Level::Error, err_msg);
        return 1;
    } catch (const std::exception& e) {
        std::string err_msg = "Error: ";
        err_msg += e.what();
        aether::Logger::log(aether::Logger::Level::Error, err_msg);
        return 1;
    }

    return 0;
}