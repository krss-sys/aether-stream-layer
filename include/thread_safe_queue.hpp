#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace aether {
template <typename T>
class ThreadSafeQueue {
   private:
    std::queue<T> q_;
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    bool closed_ = false;

   public:
    ThreadSafeQueue() = default;

    // Push an element into the queue (Thread-safe)
    void push(T item) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            q_.push(std::move(item));
        }
        cv_.notify_one();
    }

    // Try to pop an element (Blocking until an element is available or closed)
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mtx_);

        cv_.wait(lock, [&] { return !q_.empty() || closed_; });

        if (q_.empty()) {
            return std::nullopt;
        }

        T item = std::move(q_.front());
        q_.pop();
        return item;
    }

    // Close the queue and notify all waiting threads
    void close() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            closed_ = true;
        }
        cv_.notify_all();
    }

    // Pop an element with a timeout
    std::optional<T> pop_for(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mtx_);

        if (!cv_.wait_for(lock, timeout, [&] { return !q_.empty() || closed_; })) {
            return std::nullopt;
        }

        if (q_.empty()) {
            return std::nullopt;
        }

        T item = std::move(q_.front());
        q_.pop();
        return item;
    }
};
}  // namespace aether
