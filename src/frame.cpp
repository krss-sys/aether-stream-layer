#include "frame.hpp"

#include <arpa/inet.h>  // Contains htonl() for endianness conversion

#include <cstring>  // For std::memcpy
#include <span>
#include <stdexcept>  // For handling exceptions if size exceeds limits

namespace aether {

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

void FrameDecoder::feed(const uint8_t* data, size_t n) { buf_.insert(buf_.end(), data, data + n); }

std::optional<std::vector<uint8_t>> FrameDecoder::next() {
    // Check if we have enough bytes for the 4-byte length header
    if (buf_.size() < 4) {
        return std::nullopt;
    }

    uint32_t len = 0;
    std::memcpy(&len, buf_.data(), 4);
    len = ntohl(len);  // Convert from Network to Host byte order

    if (len > kMaxFrameSize) {
        error_ = true;
        return std::nullopt;
    }

    size_t total = 4 + len;
    // Wait for more data if the complete frame hasn't arrvired yet
    if (buf_.size() < total) {
        return std::nullopt;
    }

    // Extract payload and remote the processed frame from the buffer
    std::vector<uint8_t> payload(buf_.begin() + 4, buf_.begin() + total);
    buf_.erase(buf_.begin(), buf_.begin() + total);
    return payload;
}
}  // namespace aether