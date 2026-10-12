#ifndef FRAME_HPP
#define FRAME_HPP
#include <cstdint>
#include <span>
#include <vector>

// Maximum frame size limit: 1 MiB to prevent DoS attacks
constexpr uint32_t kMaxFrameSize = 1 << 20;

// Function declaration for encoding frame
std::vector<uint8_t> encode_frame(std::span<const uint8_t> payload);

#endif
