#pragma once
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace aether {
// Maximum frame size limit: 1 MiB to prevent DoS attacks
constexpr uint32_t kMaxFrameSize = 1 << 20;

// Function declaration for encoding frame
std::vector<uint8_t> encode_frame(std::span<const uint8_t> payload);

class FrameDecoder {
   private:
    std::vector<uint8_t> buf_;
    bool error_ = false;

   public:
    void feed(const uint8_t* data, size_t n);

    std::optional<std::vector<uint8_t>> next();

    bool has_error() const;
};
}  // namespace aether
