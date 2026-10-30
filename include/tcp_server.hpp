#pragma once

#include <csignal>
#include <cstdint>
#include <string>

#include "file_descriptor.hpp"
#include "thread_pool.hpp"

namespace aether {

extern volatile std::sig_atomic_t g_stop;

class TcpServer {
   public:
    // Construct a TCP server bound to a specific port
    explicit TcpServer(uint16_t port);

    // Initialize socket, set socket options, bind to port, and start listening
    void listen_start();

    FileDescriptor accept_one(std::string& peer_ip, uint16_t& peer_port);

    void run();

    void setup_signals();

   private:
    uint16_t port_;
    FileDescriptor listen_fd_;  // RAII wrapper for the listening socket file descriptor
    ThreadPool pool_{8};
};

}  // namespace aether