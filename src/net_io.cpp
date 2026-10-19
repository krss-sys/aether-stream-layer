#include "net_io.hpp"

#include <sys/socket.h>

#include <cerrno>

namespace aether {
bool send_all(int fd, const void* data, size_t len) {
    const char* ptr = static_cast<const char*>(data);
    size_t total_sent = 0;

    while (total_sent < len) {
        ssize_t n = ::send(fd, ptr + total_sent, len - total_sent, MSG_NOSIGNAL);
        if (n > 0) {
            total_sent += static_cast<size_t>(n);
        } else if (n < 0) {
            if (errno == EINTR) {
                continue;  // Interrupted by signal -> retry
            }
            return false;
        } else {
            return false;
        }
    }
    return true;
}

ssize_t recv_some(int fd, void* buf, size_t cap) {
    while (true) {
        ssize_t n = ::recv(fd, buf, cap, 0);

        if (n < 0 && errno == EINTR) {
            continue;
        }
        return n;  // Return n (> 0: data, == 0: EOF, < 0: error)
    }
}

bool recv_exact(int fd, void* buf, size_t n) {
    char* ptr = static_cast<char*>(buf);
    size_t n_left = n;

    while (n_left > 0) {
        ssize_t received = ::recv(fd, ptr, n_left, 0);

        if (received > 0) {
            ptr += received;
            n_left -=received;
        } else if (received == 0) {
            return false;
        } else {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
    }
    return true;
}
}  // namespace aether