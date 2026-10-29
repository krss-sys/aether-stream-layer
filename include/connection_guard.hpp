#pragma once
#include "logger.hpp"

namespace aether {

class ConnectionGuard {
public:
    ConnectionGuard() {
        Logger::log(Logger::Level::Info, "Client connected");
    }
    ~ConnectionGuard() {
        Logger::log(Logger::Level::Info, "Client disconnected");
    }
};

} // namespace aether