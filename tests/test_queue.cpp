#include <gtest/gtest.h>
#include "../include/thread_safe_queue.hpp"
#include <thread>
#include <vector>
#include <atomic>

// Keep existing unit tests
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

// === MPMC STRESS TEST FOR DAY 25 ===
TEST(ThreadSafeQueueTest, MPMCStressTest) {
    aether::ThreadSafeQueue<int> queue;

    const int num_producers = 4;
    const int num_consumers = 4;
    const int items_per_producer = 25000; // Each producer pushes 25,000 items
    const uint64_t total_expected = num_producers * items_per_producer;

    std::atomic<uint64_t> sent_count{0};
    std::atomic<uint64_t> received_count{0};

    // 1. Spawn 4 producer threads continuously pushing into the queue
    std::vector<std::thread> producers;
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back([&queue, &sent_count]() {
            for (int j = 0; j < items_per_producer; ++j) {
                queue.push(1);
                sent_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // 2. Spawn 4 consumer threads continuously popping from the queue
    std::vector<std::thread> consumers;
    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back([&queue, &received_count]() {
            while (true) {
                auto item = queue.pop();
                if (!item.has_value()) {
                    break; // Queue is closed and empty -> exit loop
                }
                received_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // 3. Wait for all producers to finish pushing
    for (auto& t : producers) {
        t.join();
    }

    // 4. Once all producers are done, close the queue so consumers know when to stop
    queue.close();

    // 5. Wait for all consumers to drain the remaining items and finish
    for (auto& t : consumers) {
        t.join();
    }

    // 6. Verify data integrity: Total received must strictly match total sent
    EXPECT_EQ(sent_count.load(), total_expected);
    EXPECT_EQ(received_count.load(), total_expected);
}

// === EDGE CASES & TIMEOUT TESTS FOR DAY 26 ===

// Test that calling close() correctly wakes up a consumer blocked in pop()
TEST(ThreadSafeQueueTest, CloseWakesUpBlockedConsumer) {
    aether::ThreadSafeQueue<int> queue;

    // Spawn a consumer thread that will block waiting for an item
    std::thread consumer([&queue]() {
        auto item = queue.pop();
        // Since the queue will be closed, pop() must return std::nullopt
        EXPECT_FALSE(item.has_value());
    });

    // Give the consumer a brief moment to enter the waiting state
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Close the queue while the consumer is blocked, which should trigger notify_all()
    queue.close();

    // Wait for the consumer thread to finish safely
    consumer.join();
}

// Test that pop_for correctly times out and returns std::nullopt on an empty queue
TEST(ThreadSafeQueueTest, PopForTimeoutOnEmptyQueue) {
    aether::ThreadSafeQueue<int> queue;

    auto start_time = std::chrono::steady_clock::now();
    
    // Try to pop with a 50ms timeout on an empty queue
    auto popped = queue.pop_for(std::chrono::milliseconds(50));
    
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    // Should return no value after timing out
    EXPECT_FALSE(popped.has_value());
    
    // Ensure it actually waited roughly around the specified timeout duration (allow some system jitter)
    EXPECT_GE(elapsed, 40); // At least close to 50ms
}

// Test that pop_for successfully retrieves an item if pushed before timeout expires
TEST(ThreadSafeQueueTest, PopForSuccessBeforeTimeout) {
    aether::ThreadSafeQueue<int> queue;

    // Spawn a producer thread that pushes an item after a short delay
    std::thread producer([&queue]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        queue.push(99);
    });

    // Call pop_for with a generous timeout (e.g., 200ms)
    auto popped = queue.pop_for(std::chrono::milliseconds(200));

    producer.join();

    // Must successfully retrieve the item before timeout
    ASSERT_TRUE(popped.has_value());
    EXPECT_EQ(popped.value(), 99);
}