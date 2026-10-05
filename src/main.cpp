#include <sys/socket.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <memory>
#include <system_error>
#include <thread>

#include "logger.hpp"
#include "net_io.hpp"
#include "tcp_server.hpp"

void handle_echo(aether::FileDescriptor client_fd) {
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
            // Peer closed connection
            aether::Logger::log(aether::Logger::Level::Info, "Client disconnected (peer closed)");
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

            // Format chuỗi IP:Port để in ra log
            std::string conn_msg =
                "Client connected from " + client_ip + ":" + std::to_string(client_port);
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