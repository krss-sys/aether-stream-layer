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
}  // namespace aether