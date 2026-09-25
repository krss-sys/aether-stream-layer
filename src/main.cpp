#include <chrono>
#include <iostream>
#include <thread>

#include "tcp_server.hpp"

int main() {
    try {
        aether::TcpServer server(8080);
        server.listen_start();
        std::cout << "Server is listening on port 8080..." << std::endl;

        // Keep server running for 30s so we can test with 'ss' command
        std::this_thread::sleep_for(std::chrono::seconds(30));
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}