#pragma once
#include "thread_safe_queue.hpp"
#include <cstddef>
#include <vector>
#include <thread>
#include <functional>

namespace aether {

class ThreadPool {
private:
    ThreadSafeQueue<std::function<void()>> queue_;
    std::vector<std::jthread> workers_;

public:
    explicit ThreadPool(size_t num_threads) {
        workers_.reserve(num_threads);
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this]() {
                while (auto task = queue_.pop()) {
                    (*task)();
                }
            });
        }
    }

    void submit(std::function<void()> task) {
        queue_.push(std::move(task));
    }

    ~ThreadPool() {
        queue_.close();
        workers_.clear();
    }
};
}