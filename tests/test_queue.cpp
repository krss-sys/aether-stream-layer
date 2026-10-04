#include <gtest/gtest.h>
#include "../include/thread_safe_queue.hpp"

TEST(ThreadSafeQueueTest, BasicPushPop) {
    ThreadSafeQueue<int> queue;
    queue.push(42);
    
    int value = 0;
    bool popped = queue.try_pop(value);
    
    EXPECT_TRUE(popped);
    EXPECT_EQ(value, 42);
}

TEST(ThreadSafeQueueTest, PopEmptyQueue) {
    ThreadSafeQueue<int> queue;
    int value = 0;
    
    bool popped = queue.try_pop(value);
    
    EXPECT_FALSE(popped);
}