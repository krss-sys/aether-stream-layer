#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include "net_io.hpp"

TEST(NetIoTest, RecvExactFragmentedStream) {
    int sv[2];
    // Tạo cặp socket nội bộ giả lập đường truyền mạng
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sv), 0);

    char recv_buf[8] = {0};

    // Luồng phía gửi: gửi ngắt quãng để test khả năng tích lũy của recv_exact
    std::thread sender([&sv]() {
        // Gửi trước 3 byte
        write(sv[1], "ABC", 3);
        // Ngủ một nhịp để giả lập độ trễ mạng / phân mảnh packet
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        // Gửi tiếp 5 byte còn lại
        write(sv[1], "DEFGH", 5);
        close(sv[1]);
    });

    // Phía nhận: gọi recv_exact(8) bắt buộc phải block chờ đến khi đủ 8 byte
    bool success = aether::recv_exact(sv[0], recv_buf, 8);

    sender.join();
    close(sv[0]);

    // Kiểm chứng kết quả
    EXPECT_TRUE(success);
    EXPECT_EQ(std::string(recv_buf, 8), "ABCDEFGH");
}