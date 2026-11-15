#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "file_descriptor.hpp"

namespace aether {

class AetherClient {
   private:
    FileDescriptor m_sockfd;

   public:
    AetherClient() = default;

    ~AetherClient();

    AetherClient(const AetherClient&) = delete;
    AetherClient& operator=(const AetherClient&) = delete;

    AetherClient(AetherClient&& other) noexcept;

    AetherClient& operator=(AetherClient&& other) noexcept;

    bool connect(const std::string& host, uint16_t port);

    void disconnect();

    bool send_frame(std::span<const uint8_t> frame_data);

    std::optional<std::vector<uint8_t>> recv_frame();
};
}  // namespace aether
