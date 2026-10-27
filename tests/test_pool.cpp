#include "thread_pool.hpp"
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>

TEST(ThreadPoolTest, Submit1000TasksAndVerify) {
    aether::ThreadPool pool(4);
    std::atomic<int> counter{0};

    for (int i = 0; i < 1000; ++i) {
        pool.submit([&counter]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }

    // Wait for all tasks to complete by letting pool go out of scope or sleeping briefly
    // Destructor will join threads after queue is closed and drained
    // Here we can also loop until counter reaches 1000 to avoid arbitrary sleep
    auto start = std::chrono::steady_clock::now();
    while (counter.load() < 1000) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > 5) {
            break;
        }
    }

    EXPECT_EQ(counter.load(), 1000);
}

TEST(ThreadPoolTest, SingleThreadAndImmediateDestruction) {
    {
        aether::ThreadPool pool(1);
        std::atomic<int> flag{0};
        pool.submit([&flag]() {
            flag.store(1, std::memory_order_relaxed);
        });
    } 

    {
        aether::ThreadPool pool(2);
        for (int i = 0; i < 10; ++i) {
            pool.submit([]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            });
        }
    } 

    SUCCEED();
}