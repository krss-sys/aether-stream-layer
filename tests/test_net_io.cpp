#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <thread>

#include "net_io.hpp"

TEST(NetIoTest, RecvExactFragmentedStream) {
    int sv[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sv), 0);

    char recv_buf[8] = {0};

    std::thread sender([&sv]() {
        write(sv[1], "ABC", 3);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        write(sv[1], "DEFGH", 5);
        close(sv[1]);
    });

    bool success = aether::recv_exact(sv[0], recv_buf, 8);

    sender.join();
    close(sv[0]);

    EXPECT_TRUE(success);
    EXPECT_EQ(std::string(recv_buf, 8), "ABCDEFGH");
}