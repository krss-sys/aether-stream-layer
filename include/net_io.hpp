#pragma once

#include <sys/types.h>

#include <cstddef>

namespace aether {
bool send_all(int fd, const void* data, size_t len);

ssize_t recv_some(int fd, void* buf, size_t cap);

bool recv_exact(int fd, void* buf, size_t n);
}  // namespace aether