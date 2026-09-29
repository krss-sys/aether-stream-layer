#include <sys/socket.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <system_error>

#include "tcp_server.hpp"

int main() {
    try {
        // Initialize TCP server bound to port 8080
        aether::TcpServer server(8080);

        // Start listening for incoming connections
        server.listen_start();
        std::cout << "Server is listening on port 8080..." << std::endl;

        std::string client_ip;
        uint16_t client_port = 0;

        // Block and wait to accept a single client connection
        aether::FileDescriptor client_fd = server.accept_one(client_ip, client_port);
        std::cout << "Client connected from " << client_ip << ":" << client_port << std::endl;

        char buffer[1024];
        while (true) {
            ssize_t bytes_received = ::recv(client_fd.get(), buffer, sizeof(buffer), 0);

            if (bytes_received > 0) {
                // n > 0: Send back the received data to client
                ::send(client_fd.get(), buffer, bytes_received, 0);
            } else if (bytes_received == 0) {
                // n == 0: Peer closed connection
                std::cout << "peer closed" << std::endl;
                break;
            } else {
                // n < 0: Error occurred
                std::cerr << "recv error: " << std::strerror(errno) << std::endl;
                break;
            }
        }

    } catch (const std::system_error& e) {
        std::cerr << "System error: " << e.what() << " (code: " << e.code() << ")" << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}