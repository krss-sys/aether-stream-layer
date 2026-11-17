#include "frame.hpp"

#include <arpa/inet.h>

#include <cstring>
#include <span>
#include <stdexcept>
#include <cstdint>

namespace aether {

std::vector<uint8_t> encode_frame(std::span<const uint8_t> payload) {
    if (payload.size() > aether::kMaxFrameSize) {
        throw std::runtime_error("Payload size exceeds maximum frame size limit (1 MiB)");
    }

    auto payload_len = static_cast<uint32_t>(payload.size());

    uint32_t net_len = htonl(payload_len);

    std::vector<uint8_t> frame;
    frame.resize(4 + payload.size());

    std::memcpy(frame.data(), &net_len, 4);

    if (!payload.empty()) {
        std::memcpy(frame.data() + 4, payload.data(), payload.size());
    }

    return frame;
}

void FrameDecoder::feed(const uint8_t* data, size_t n) { buf_.insert(buf_.end(), data, data + n); }

std::optional<std::vector<uint8_t>> FrameDecoder::next() {
    if (buf_.size() < 4) {
        return std::nullopt;
    }

    uint32_t len = 0;
    std::memcpy(&len, buf_.data(), 4);
    len = ntohl(len);

    if (len > aether::kMaxFrameSize) {
        error_ = true;
        return std::nullopt;
    }

    size_t total = 4 + len;
    if (buf_.size() < total) {
        return std::nullopt;
    }

    std::vector<uint8_t> payload(buf_.begin() + 4, buf_.begin() + static_cast<ptrdiff_t>(total));
    buf_.erase(buf_.begin(), buf_.begin() + static_cast<ptrdiff_t>(total));
    return payload;
}

bool FrameDecoder::has_error() const { return error_; }
}  // namespace aether
