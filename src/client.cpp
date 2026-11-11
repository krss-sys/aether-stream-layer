#include "client.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "frame.hpp"
#include "net_io.hpp"

namespace aether {
// AetherClient implementation

AetherClient::~AetherClient() { disconnect(); }

AetherClient::AetherClient(AetherClient&& other) noexcept : m_sockfd(other.m_sockfd) {
    other.m_sockfd = -1;
}

AetherClient& AetherClient::operator=(AetherClient&& other) noexcept {
    if (this != &other) {
        disconnect();
        m_sockfd = other.m_sockfd;
        other.m_sockfd = -1;
    }
    return *this;
}

bool AetherClient::connect(const std::string& host, uint16_t port) {
    m_sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockfd < 0) {
        return false;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        ::close(m_sockfd);
        m_sockfd = -1;
        return false;
    }

    if (::connect(m_sockfd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        ::close(m_sockfd);
        m_sockfd = -1;
        return false;
    }

    return true;
}

void AetherClient::disconnect() {
    if (m_sockfd >= 0) {
        ::close(m_sockfd);
        m_sockfd = -1;
    }
}

bool AetherClient::send_frame(std::span<const uint8_t> frame_data) {
    if (m_sockfd < 0) return false;

    uint32_t payload_len = static_cast<uint32_t>(frame_data.size());
    if (payload_len > aether::kMaxFrameSize) {
        return false;
    }

    uint32_t net_len = htonl(payload_len);
    if (!send_all(m_sockfd, &net_len, sizeof(net_len))) {
        return false;
    }

    if (payload_len > 0) {
        if (!send_all(m_sockfd, frame_data.data(), payload_len)) {
            return false;
        }
    }

    return true;
}

std::optional<std::vector<uint8_t>> AetherClient::recv_frame() {
    if (m_sockfd < 0) return std::nullopt;

    uint32_t net_len = 0;
    if (!recv_exact(m_sockfd, &net_len, sizeof(net_len))) {
        return std::nullopt;
    }

    uint32_t len = ntohl(net_len);
    if (len > aether::kMaxFrameSize) {
        disconnect();
        return std::nullopt;
    }

    std::vector<uint8_t> payload(len);
    if (!recv_exact(m_sockfd, payload.data(), len)) {
        return std::nullopt;
    }

    return payload;
}

}  // namespace aether