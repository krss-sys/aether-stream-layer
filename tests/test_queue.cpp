#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include "thread_safe_queue.hpp"

TEST(ThreadSafeQueueTest, BasicPushPop) {
    aether::ThreadSafeQueue<int> queue;
    queue.push(42);

    auto popped = queue.pop();

    ASSERT_TRUE(popped.has_value());
    EXPECT_EQ(popped.value(), 42);
}

TEST(ThreadSafeQueueTest, PopEmptyQueueWithTimeout) {
    aether::ThreadSafeQueue<int> queue;

    auto popped = queue.pop_for(std::chrono::milliseconds(50));

    EXPECT_FALSE(popped.has_value());
}

TEST(ThreadSafeQueueTest, MPMCStressTest) {
    aether::ThreadSafeQueue<int> queue;

    const int num_producers = 4;
    const int num_consumers = 4;
    const int items_per_producer = 25000;
    const uint64_t total_expected = num_producers * items_per_producer;

    std::atomic<uint64_t> sent_count{0};
    std::atomic<uint64_t> received_count{0};

    std::vector<std::thread> producers;
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back([&queue, &sent_count]() {
            for (int j = 0; j < items_per_producer; ++j) {
                queue.push(1);
                sent_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    std::vector<std::thread> consumers;
    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back([&queue, &received_count]() {
            while (true) {
                auto item = queue.pop();
                if (!item.has_value()) {
                    break;
                }
                received_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : producers) {
        t.join();
    }

    queue.close();

    for (auto& t : consumers) {
        t.join();
    }

    EXPECT_EQ(sent_count.load(), total_expected);
    EXPECT_EQ(received_count.load(), total_expected);
}

TEST(ThreadSafeQueueTest, CloseWakesUpBlockedConsumer) {
    aether::ThreadSafeQueue<int> queue;

    std::thread consumer([&queue]() {
        auto item = queue.pop();
        EXPECT_FALSE(item.has_value());
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    queue.close();

    consumer.join();
}

TEST(ThreadSafeQueueTest, PopForTimeoutOnEmptyQueue) {
    aether::ThreadSafeQueue<int> queue;

    auto start_time = std::chrono::steady_clock::now();

    auto popped = queue.pop_for(std::chrono::milliseconds(50));

    auto end_time = std::chrono::steady_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    EXPECT_FALSE(popped.has_value());

    EXPECT_GE(elapsed, 40);
}

TEST(ThreadSafeQueueTest, PopForSuccessBeforeTimeout) {
    aether::ThreadSafeQueue<int> queue;

    std::thread producer([&queue]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        queue.push(99);
    });

    auto popped = queue.pop_for(std::chrono::milliseconds(200));

    producer.join();

    ASSERT_TRUE(popped.has_value());
    EXPECT_EQ(popped.value(), 99);
}