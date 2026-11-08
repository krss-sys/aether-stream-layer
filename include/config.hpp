#pragma once
#include <cstddef>
#include <cstdint>

namespace aether::config {
    constexpr uint16_t kDefaultPort = 8080;
    constexpr std::size_t kWorkerCount = 8;
    constexpr std::size_t kRecvBufferSize = 4096;
    constexpr int kRecvTimeoutSec = 1;
    constexpr int kPollTimeoutMs = 500;
}