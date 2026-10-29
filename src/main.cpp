#include "logger.hpp"
#include "tcp_server.hpp"

int main() {
    try {
        aether::TcpServer server(8080);
        server.run(); 
    } catch (const std::exception& e) {
        aether::Logger::log(aether::Logger::Level::Error, e.what());
        return 1;
    }
    return 0;
}