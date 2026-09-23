#ifndef AETHER_FD_HPP
#define AETHER_FD_HPP

#include <unistd.h>

#include <utility>

namespace aether {
// RAII wrapper for linux file descriptor
class FileDescriptor {
   private:
    int fd_;

   public:
    // Create FileDescriptor with raw fd (default is -1)
    explicit FileDescriptor(int fd = -1) : fd_(fd) {}

    // Automatically close fd when object is destroyed
    ~FileDescriptor() {
        if (fd_ >= 0) {
            ::close(fd_);
        }
    }

    // Disable copy (one fd belongs to one object only)
    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    // Move constructor: Transfer ownership form another object
    FileDescriptor(FileDescriptor&& other) noexcept : fd_(other.release()) {}

    // Move assignment: Close old fd and take ownership from other
    FileDescriptor& operator=(FileDescriptor&& other) noexcept {
        if (this != &other) {
            reset(other.release());
        }
        return *this;
    }

    // Return internal raw fd number
    int get() const { return fd_; }

    bool valid() const { return fd_ >= 0; }

    int release() {
        int temp = fd_;
        fd_ = -1;
        return temp;
    }

    void reset(int new_fd = -1) {
        if (fd_ >= 0) {
            ::close(fd_);
        }
        fd_ = new_fd;
    }
};
}  // namespace aether

#endif