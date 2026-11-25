#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace aether {
enum class Command : uint8_t { kProduce = 1, kFetch = 2, kCommit = 3, kPing = 4 };
enum class StatusCode : uint8_t { kOk = 0, kBusy = 1, kBadRequest = 2 };

struct Request {
    Command cmd;
    std::string topic;
    std::vector<uint8_t> body;
};

struct Response {
    StatusCode status;
    std::vector<uint8_t> body;
};

std::optional<Request> decode_request(std::span<const uint8_t> payload);
std::vector<uint8_t> encode_response(StatusCode status, std::span<const uint8_t> body);

std::vector<uint8_t> encode_request(const Request& req);
std::optional<Response> decode_response(std::span<const uint8_t> payload);

class BodyReader {
   public:
    explicit BodyReader(std::span<const uint8_t> data);

    bool read_u8(uint8_t& out);
    bool read_u16(uint16_t& out);
    bool read_u32(uint32_t& out);
    bool read_u64(uint64_t& out);
    bool read_bytes(size_t n, std::span<const uint8_t>& out);

    bool at_end() const;
    size_t remaining() const;

   private:
    std::span<const uint8_t> data_;
    size_t pos_ = 0;
};
}  // namespace aether