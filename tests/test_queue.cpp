#include <gtest/gtest.h>

#include "../include/thread_safe_queue.hpp"

TEST(ThreadSafeQueueTest, BasicPushPop) {
    aether::ThreadSafeQueue<int> queue;
    // Push one element into the queue
    queue.push(42);

    // Pop the element back out
    auto popped = queue.pop();

    // The value should exist and match what was pushed
    ASSERT_TRUE(popped.has_value());
    EXPECT_EQ(popped.value(), 42);
}

TEST(ThreadSafeQueueTest, PopEmptyQueueWithTimeout) {
    aether::ThreadSafeQueue<int> queue;

    // Try to pop from an empty queue with a short timeout
    auto popped = queue.pop_for(std::chrono::milliseconds(50));

    // Should return empty since nothing was pushed
    EXPECT_FALSE(popped.has_value());
}