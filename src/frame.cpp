#include "frame.hpp"

#include <arpa/inet.h>  // Contains htonl() for endianness conversion
#include <span>
#include <cstring>    // For std::memcpy
#include <stdexcept>  // For handling exceptions if size exceeds limits

std::vector<uint8_t> encode_frame(std::span<const uint8_t> payload) {
    // Check size limit (Mitigate DoS attacks > 1 MiB)
    if (payload.size() > kMaxFrameSize) {
        throw std::runtime_error("Payload size exceeds maximum frame size limit (1 MiB)");
    }

    uint32_t payload_len = static_cast<uint32_t>(payload.size());

    // Convert length to Network Byte Order (Big-Endian) using htonl()
    uint32_t net_len = htonl(payload_len);

    // Create result vector: first 4 bytes for Length, followed by payload
    std::vector<uint8_t> frame;
    frame.resize(4 + payload.size());

    // Copy the 4-byte Big-Endian length into the beginning of the vector
    std::memcpy(frame.data(), &net_len, 4);

    // Copy payload bytes right after the length header
    if (!payload.empty()) {
        std::memcpy(frame.data() + 4, payload.data(), payload.size());
    }

    return frame;
}