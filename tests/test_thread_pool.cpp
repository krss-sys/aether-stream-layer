#include "thread_pool.hpp"
#include <gtest/gtest.h> 
#include <atomic>
#include <chrono>

TEST(ThreadPoolTest, SubmitMultipleTasks) {
    aether::ThreadPool pool(4);
    std::atomic<int> count{0};

    for (int i = 0; i < 10; ++i) {
        pool.submit([&count]() {
            count.fetch_add(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
    }

    // Cho pool thời gian xử lý xong các task
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_EQ(count.load(), 10);
}