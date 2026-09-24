#include <gtest/gtest.h>
#include <unistd.h>
#include <cstring>
#include "fd.hpp"

// Test 1: Verify RAII wrapper over pipe(), writing and reading data
TEST(FileDescriptorTest, PipeReadWrite) {
    int fds[2];
    // ::pipe returns 0 on success; fds[0] is read end, fds[1] is write end
    ASSERT_EQ(::pipe(fds), 0);

    aether::FileDescriptor read_fd(fds[0]);
    aether::FileDescriptor write_fd(fds[1]);

    EXPECT_TRUE(read_fd.valid());
    EXPECT_TRUE(write_fd.valid());

    const char* msg = "hello aether";
    ssize_t bytes_written = ::write(write_fd.get(), msg, strlen(msg));
    EXPECT_EQ(bytes_written, static_cast<ssize_t>(strlen(msg)));

    char buffer[32] = {0};
    ssize_t bytes_read = ::read(read_fd.get(), buffer, sizeof(buffer));
    EXPECT_EQ(bytes_read, static_cast<ssize_t>(strlen(msg)));
    EXPECT_STREQ(buffer, msg);
}

// Test 2: Verify Move Semantics and release() method behavior
TEST(FileDescriptorTest, MoveAndRelease) {
    int fds[2];
    ASSERT_EQ(::pipe(fds), 0);

    aether::FileDescriptor fd1(fds[0]);
    int raw_fd = fd1.get();

    // Move constructor: fd2 acquires ownership, fd1 is reset to -1
    aether::FileDescriptor fd2(std::move(fd1));
    EXPECT_EQ(fd1.get(), -1);
    EXPECT_FALSE(fd1.valid());
    EXPECT_EQ(fd2.get(), raw_fd);

    // Release: fd2 relinquishes ownership without closing raw_fd in OS
    int released_fd = fd2.release();
    EXPECT_EQ(released_fd, raw_fd);
    EXPECT_EQ(fd2.get(), -1);

    // Explicitly close the released fd to prevent resource leaks
    ::close(released_fd);
    ::close(fds[1]); // Close remaining write end
}