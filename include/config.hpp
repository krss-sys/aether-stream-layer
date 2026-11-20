#pragma once
#include <cstddef>
#include <cstdint>

namespace aether::config {

constexpr uint16_t kDefaultPort = 8080;
constexpr std::size_t kWorkerCount = 8;
constexpr std::size_t kRecvBufferSize = 4096;
constexpr int kRecvTimeoutSec = 1;
constexpr int kPollTimeoutMs = 500;

constexpr std::size_t kMaxFrameSize = 1 << 20;  // 1 MiB

constexpr std::size_t kMaxMessageSize = kMaxFrameSize - 128;

constexpr std::size_t kMaxTopics = 64;
constexpr std::size_t kMaxGroups = 1024;
constexpr std::size_t kMaxFetchMessages = 1000;

static_assert(kMaxMessageSize + 17 <= kMaxFrameSize,
              "FETCH response header does not fit in a frame");
static_assert(kMaxMessageSize + 3 + 64 <= kMaxFrameSize,
              "PRODUCE request header does not fit in a frame");

}  // namespace aether::config