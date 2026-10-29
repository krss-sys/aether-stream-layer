#pragma once

#include <string>
#include <vector>
#include <optional>
#include <span>
#include <cstdint>
#include <memory>
#include "file_descriptor.hpp"

namespace aether {

// Handle incoming client echo requests
void handle_echo(std::shared_ptr<FileDescriptor> client_fd);

// TCP client wrapper class for connecting and framing communication
class AetherClient {
private:
    int m_sockfd = -1;
    // Maximum allowed frame payload size (1 MB)
    static constexpr size_t kMaxFrameSize = 1024 * 1024;

public:
    AetherClient() = default;
    
    // Close the socket automatically on destruction
    ~AetherClient();

    // Disable copying (socket ownership must be unique)
    AetherClient(const AetherClient&) = delete;
    AetherClient& operator=(const AetherClient&) = delete;

    // Move constructor: take over the socket and invalidate the source
    AetherClient(AetherClient&& other) noexcept;
    
    // Move assignment: release current socket, then take over the other's
    AetherClient& operator=(AetherClient&& other) noexcept;

    // Connect to the server at host:port; return false on any failure
    bool connect(const std::string& host, uint16_t port);

    // Close the socket if it is open
    void disconnect();

    // Send one frame: 4-byte length prefix followed by the payload
    bool send_frame(std::span<const uint8_t> frame_data);

    // Receive one frame: read the length prefix, then the payload
    std::optional<std::vector<uint8_t>> recv_frame();
};

} // namespace aether