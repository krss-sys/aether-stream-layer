#ifndef AETHER_NET_IO_HPP
#define AETHER_NET_IO_HPP

#include <sys/types.h>

#include <cstddef>

namespace aether {
// Sends all data (handles partial sends, MSG_NOSIGNAL, and EINTR)
bool send_all(int fd, const void* data, size_t len);

// Receives data safely (retries on EINTR)
ssize_t recv_some(int fd, void* buf, size_t cap);
}  // namespace aether
#endif