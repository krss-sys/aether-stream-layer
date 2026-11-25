#include "message.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

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

bool BodyReader::at_end() const {
    return pos_ >= data_.size();
}

size_t BodyReader::remaining() const {
    return data_.size() - pos_;
}
}  // namespace aether