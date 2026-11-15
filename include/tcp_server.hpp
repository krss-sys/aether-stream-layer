#pragma once

#include <cstdint>
#include <string>

#include "config.hpp"
#include "file_descriptor.hpp"
#include "thread_pool.hpp"

namespace aether {

class TcpServer {
   public:
    explicit TcpServer(uint16_t port);

    void listen_start();

    FileDescriptor accept_one(std::string& peer_ip, uint16_t& peer_port);

    void run();

    void setup_signals();

   private:
    uint16_t port_;
    FileDescriptor listen_fd_;
    ThreadPool pool_{aether::config::kWorkerCount};
};

}  // namespace aether