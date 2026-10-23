#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <thread>

#include "net_io.hpp"

TEST(NetIoTest, RecvExactFragmentedStream) {
    int sv[2];
    // Create a socket pair to simulate a network connection
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sv), 0);

    char recv_buf[8] = {0};

    // Sender thread: write in chunks to test recv_exact's accumulation
    std::thread sender([&sv]() {
        // Send the first 3 bytes
        write(sv[1], "ABC", 3);
        // Sleep a bit to simulate network delay / packet fragmentation
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        // Send the remaining 5 bytes
        write(sv[1], "DEFGH", 5);
        close(sv[1]);
    });

    // Receiver side: recv_exact(8) must block until all 8 bytes arrive
    bool success = aether::recv_exact(sv[0], recv_buf, 8);

    sender.join();
    close(sv[0]);

    // Verify the result
    EXPECT_TRUE(success);
    EXPECT_EQ(std::string(recv_buf, 8), "ABCDEFGH");
}