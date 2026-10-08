#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "file_descriptor.hpp"
#include "logger.hpp"
#include "net_io.hpp"

int main(int argc, char* argv[]) {
    // 1. Validate command-line arguments
    if (argc < 4) {
        aether::Logger::log(aether::Logger::Level::Error,
                            "Usage: ./aether_cli <ip> <port> <message>");
        return EXIT_FAILURE;
    }

    std::string ip = argv[1];
    uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));
    std::string message = argv[3];

    // 2. Create client socket
    int raw_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (raw_fd < 0) {
        aether::Logger::log(aether::Logger::Level::Error,
                            std::string("Failed to create socket: ") + strerror(errno));
        return EXIT_FAILURE;
    }

    // Wrap raw_fd with FileDescriptor for RAII automatic cleanup
    aether::FileDescriptor fd(raw_fd);

    // 3. Initialize server address structure (sockaddr_in)
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Convert IP string to binary format using inet_pton
    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
        aether::Logger::log(aether::Logger::Level::Error, std::string("Invalid IP address: ") + ip);
        return EXIT_FAILURE;
    }

    // 4. Initiate connection to the server
    if (connect(fd.get(), reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        aether::Logger::log(aether::Logger::Level::Error, std::string("Connect failed to ") + ip +
                                                              ":" + std::to_string(port) + ": " +
                                                              strerror(errno));
        return EXIT_FAILURE;
    }

    aether::Logger::log(aether::Logger::Level::Info, std::string("Connected successfully to ") +
                                                         ip + ":" + std::to_string(port));

    // 5. Send message to server using send_all(fd, data, len)
    if (!aether::send_all(fd.get(), message.data(), message.size())) {
        aether::Logger::log(aether::Logger::Level::Error, "Failed to send full message to server");
        return EXIT_FAILURE;
    }

    // 6. Receive echo response from server using recv_some(fd, buf, cap)
    char buffer[1024];
    ssize_t bytes_received = aether::recv_some(fd.get(), buffer, sizeof(buffer) - 1);
    if (bytes_received <= 0) {
        aether::Logger::log(aether::Logger::Level::Error,
                            "Failed to receive response or connection closed");
        return EXIT_FAILURE;
    }

    // Null-terminate the buffer and print to stdout
    buffer[bytes_received] = '\0';
    std::cout << buffer << std::endl;

    return EXIT_SUCCESS;
}