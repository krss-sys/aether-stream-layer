#pragma once

#include <cstdint>

#include "file_descriptor.hpp"

namespace aether {

class TcpServer {
   public:
    // Construct a TCP server bound to a specific port
    explicit TcpServer(uint16_t port);

    // Initialize socket, set socket options, bind to port, and start listening
    void listen_start();

   private:
    uint16_t port_;
    FileDescriptor listen_fd_;  // RAII wrapper for the listening socket file descriptor
};

}  // namespace aether