#include "message.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace aether {
BodyReader::BodyReader(std::span<const uint8_t> data) : data_(data), pos_(0) {}

bool BodyReader::read_u8(uint8_t& out) {
    if (pos_ >= data_.size()) return false;

    out = data_[pos_];
    ++pos_;
    return true;
}

bool BodyReader::read_u16(uint16_t& out) {
    if (pos_ > data_.size() || data_.size() - pos_ < 2) return false;

    uint16_t value = static_cast<uint16_t>(data_[pos_]) << 8;
    value |= static_cast<uint16_t>(data_[pos_ + 1]);
    out = value;
    pos_ += 2;
    return true;
}

bool BodyReader::read_u32(uint32_t& out) {
    if (pos_ > data_.size() || data_.size() - pos_ < 4) return false;

    uint32_t value = static_cast<uint32_t>(data_[pos_]) << 24;
    value |= static_cast<uint32_t>(data_[pos_ + 1]) << 16;
    value |= static_cast<uint32_t>(data_[pos_ + 2]) << 8;
    value |= static_cast<uint32_t>(data_[pos_ + 3]);
    out = value;
    pos_ += 4;
    return true;
}

bool BodyReader::read_u64(uint64_t& out) {
    if (pos_ > data_.size() || data_.size() - pos_ < 8) return false;

    uint64_t value = static_cast<uint64_t>(data_[pos_]) << 56;
    value |= static_cast<uint64_t>(data_[pos_ + 1]) << 48;
    value |= static_cast<uint64_t>(data_[pos_ + 2]) << 40;
    value |= static_cast<uint64_t>(data_[pos_ + 3]) << 32;
    value |= static_cast<uint64_t>(data_[pos_ + 4]) << 24;
    value |= static_cast<uint64_t>(data_[pos_ + 5]) << 16;
    value |= static_cast<uint64_t>(data_[pos_ + 6]) << 8;
    value |= static_cast<uint64_t>(data_[pos_ + 7]);
    out = value;
    pos_ += 8;
    return true;
}

bool BodyReader::read_bytes(size_t n, std::span<const uint8_t>& out) {
    if (pos_ > data_.size() || data_.size() - pos_ < n) return false;

    out = data_.subspan(pos_, n);
    pos_ += n;
    return true;
}

bool BodyReader::at_end() const { return pos_ >= data_.size(); }

size_t BodyReader::remaining() const { return data_.size() - pos_; }

static bool valid_topic_name(std::span<const uint8_t> name) {
    if (name.empty() || name.size() > 64) return false;

    for (uint8_t c : name) {
        bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
                  c == '.' || c == '_' || c == '-';
        if (!ok) return false;
    }
    return true;
}

static void append_u16_be(std::vector<uint8_t>& out, uint16_t value) {
    out.push_back(static_cast<uint8_t>(value >> 8));
    out.push_back(static_cast<uint8_t>(value));
}

static void append_u32_be(std::vector<uint8_t>& out, uint32_t value) {
    out.push_back(static_cast<uint8_t>(value >> 24));
    out.push_back(static_cast<uint8_t>(value >> 16));
    out.push_back(static_cast<uint8_t>(value >> 8));
    out.push_back(static_cast<uint8_t>(value));
}

static void append_u64_be(std::vector<uint8_t>& out, uint64_t value) {
    out.push_back(static_cast<uint8_t>(value >> 56));
    out.push_back(static_cast<uint8_t>(value >> 48));
    out.push_back(static_cast<uint8_t>(value >> 40));
    out.push_back(static_cast<uint8_t>(value >> 32));
    out.push_back(static_cast<uint8_t>(value >> 24));
    out.push_back(static_cast<uint8_t>(value >> 16));
    out.push_back(static_cast<uint8_t>(value >> 8));
    out.push_back(static_cast<uint8_t>(value));
}

static void append_bytes(std::vector<uint8_t>& out, std::span<const uint8_t> bytes) {
    out.insert(out.end(), bytes.begin(), bytes.end());
}

std::optional<Request> decode_request(std::span<const uint8_t> payload) {
    BodyReader reader(payload);

    uint8_t cmd_byte = 0;
    if (!reader.read_u8(cmd_byte)) return std::nullopt;

    uint16_t topic_len = 0;
    if (!reader.read_u16(topic_len)) return std::nullopt;

    if (topic_len > 64) return std::nullopt;

    std::span<const uint8_t> topic_bytes;
    if (!reader.read_bytes(topic_len, topic_bytes)) return std::nullopt;

    const bool is_ping = (cmd_byte == static_cast<uint8_t>(Command::kPing));

    if (is_ping) {
        // PING: topic rỗng được phép; nếu có topic thì vẫn phải hợp lệ.
        if (!topic_bytes.empty() && !valid_topic_name(topic_bytes)) {
            return std::nullopt;
        }
    } else {
        // Các lệnh khác: topic phải hợp lệ và không rỗng.
        if (topic_bytes.empty() || !valid_topic_name(topic_bytes)) {
            return std::nullopt;
        }
    }

    std::string topic(reinterpret_cast<const char*>(topic_bytes.data()), topic_bytes.size());

    std::vector<uint8_t> body(topic_bytes.end(), payload.end());

    return Request{static_cast<Command>(cmd_byte), std::move(topic), std::move(body)};
}

std::vector<uint8_t> encode_request(const Request& req) {
    std::vector<uint8_t> out;

    out.push_back(static_cast<uint8_t>(req.cmd));

    append_u16_be(out, static_cast<uint16_t>(req.topic.size()));

    append_bytes(out, std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(req.topic.data()),
                                               req.topic.size()));

    append_bytes(out, std::span<const uint8_t>(req.body));

    return out;
}

std::vector<uint8_t> encode_response(StatusCode status, std::span<const uint8_t> body) {
    std::vector<uint8_t> out;
    out.push_back(static_cast<uint8_t>(status));
    append_bytes(out, body);
    return out;
}

std::optional<Response> decode_response(std::span<const uint8_t> payload) {
    BodyReader reader(payload);

    uint8_t status_byte = 0;
    if (!reader.read_u8(status_byte)) return std::nullopt;

    std::vector<uint8_t> body(payload.begin() + 1, payload.end());

    return Response{static_cast<StatusCode>(status_byte), std::move(body)};
}
}  // namespace aether