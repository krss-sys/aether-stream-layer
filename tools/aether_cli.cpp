#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "client.hpp"
#include "logger.hpp"

int main(int argc, char* argv[]) {
    // 1. Validate command-line arguments
    if (argc < 4) {
        aether::Logger::log(aether::Logger::Level::Error,
                            "Usage: ./aether_cli <ip> <port> <message>");
        return EXIT_FAILURE;
    }

    std::string ip_address = argv[1];

    int port_int = 0;
    try {
        port_int = std::stoi(argv[2]);
    } catch (const std::invalid_argument&) {
        std::cerr << "Error: port must be a number\n";
        return 1;
    } catch (const std::out_of_range&) {
        std::cerr << "Error: port out of range\n";
        return 1;
    }
    if (port_int < 1 || port_int > 65535) {
        std::cerr << "Error: port must be between 1 and 65535\n";
        return 1;
    }
    uint16_t port = static_cast<uint16_t>(port_int);

    std::string message = argv[3];

    // 2. Initialize AetherClient (automatic socket lifecycle management via RAII)
    aether::AetherClient client;

    // 3. Connect to the server/broker
    if (!client.connect(ip_address, port)) {
        aether::Logger::log(
            aether::Logger::Level::Error,
            std::string("Connect failed to ") + ip_address + ":" + std::to_string(port));
        return EXIT_FAILURE;
    }

    aether::Logger::log(aether::Logger::Level::Info, std::string("Connected successfully to ") +
                                                         ip_address + ":" + std::to_string(port));

    // 4. Wrap message into byte vector to send via send_frame
    std::vector<uint8_t> payload(message.begin(), message.end());
    if (!client.send_frame(payload)) {
        aether::Logger::log(aether::Logger::Level::Error, "Failed to send frame to server");
        return EXIT_FAILURE;
    }

    // 5. Receive response frame from server via recv_frame
    auto response = client.recv_frame();
    if (!response.has_value()) {
        aether::Logger::log(aether::Logger::Level::Error,
                            "Failed to receive response or connection closed by server");
        return EXIT_FAILURE;
    }

    // Print the received result to stdout
    std::string output(response.value().begin(), response.value().end());
    std::cout << output << std::endl;

    return EXIT_SUCCESS;
}