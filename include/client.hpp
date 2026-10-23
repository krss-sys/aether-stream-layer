#pragma once
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include "net_io.hpp"

namespace aether {
class AetherClient {
   private:
    int m_sockfd = -1;
    // Maximum allowed frame payload size (1 MB)
    static constexpr size_t kMaxFrameSize = 1024 * 1024;

   public:
    AetherClient() = default;

    // Close the socket automatically on destruction
    ~AetherClient() { disconnect(); }

    // Disable copying (socket ownership must be unique)
    AetherClient(const AetherClient&) = delete;
    AetherClient& operator=(const AetherClient&) = delete;

    // Move constructor: take over the socket and invalidate the source
    AetherClient(AetherClient&& other) noexcept : m_sockfd(other.m_sockfd) { other.m_sockfd = -1; }

    // Move assignment: release current socket, then take over the other's
    AetherClient& operator=(AetherClient&& other) noexcept {
        if (this != &other) {
            disconnect();
            m_sockfd = other.m_sockfd;
            other.m_sockfd = -1;
        }
        return *this;
    }

    // Connect to the server at host:port; return false on any failure
    bool connect(const std::string& host, uint16_t port) {
        m_sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (m_sockfd < 0) {
            return false;
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        // Convert the host string to a binary IPv4 address
        if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
            ::close(m_sockfd);
            m_sockfd = -1;
            return false;
        }

        // Establish the TCP connection
        if (::connect(m_sockfd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) <
            0) {
            ::close(m_sockfd);
            m_sockfd = -1;
            return false;
        }
        return true;
    }

    // Close the socket if it is open
    void disconnect() {
        if (m_sockfd >= 0) {
            ::close(m_sockfd);
            m_sockfd = -1;
        }
    }

    // Send one frame: 4-byte length prefix followed by the payload
    bool send_frame(std::span<const uint8_t> frame_data) {
        if (m_sockfd < 0) return false;

        uint32_t payload_len = static_cast<uint32_t>(frame_data.size());
        // Reject frames that exceed the max size
        if (payload_len > kMaxFrameSize) {
            return false;
        }

        // Convert length to network byte order
        uint32_t net_len = htonl(payload_len);

        // Send the length prefix first
        if (!send_all(m_sockfd, &net_len, sizeof(net_len))) {
            return false;
        }

        // Then send the payload (if any)
        if (payload_len > 0) {
            if (!send_all(m_sockfd, frame_data.data(), payload_len)) {
                return false;
            }
        }

        return true;
    }

    // Receive one frame: read the length prefix, then the payload
    std::optional<std::vector<uint8_t>> recv_frame() {
        if (m_sockfd < 0) return std::nullopt;

        uint32_t net_len = 0;
        // Read the 4-byte length prefix
        if (!recv_exact(m_sockfd, &net_len, sizeof(net_len))) {
            return std::nullopt;
        }

        // Convert back to host byte order
        uint32_t len = ntohl(net_len);

        // Reject oversized frames and drop the connection (defense)
        if (len > kMaxFrameSize) {
            disconnect();
            return std::nullopt;
        }

        // Read exactly len bytes of payload
        std::vector<uint8_t> payload(len);
        if (!recv_exact(m_sockfd, payload.data(), len)) {
            return std::nullopt;
        }

        return payload;
    }
};
}  // namespace aether