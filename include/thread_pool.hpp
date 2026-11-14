#pragma once
#include <cstddef>
#include <exception>
#include <functional>
#include <thread>
#include <vector>

#include "logger.hpp"
#include "thread_safe_queue.hpp"

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
                    try {
                        (*task)();
                    } catch (const std::exception& e) {
                        Logger::log(Logger::Level::Error,
                                    std::string("Worker caught exception: ") + e.what());
                    } catch (...) {
                        Logger::log(Logger::Level::Error, "Worker caught unknown exception");
                    }
                }
            });
        }
    }

    void submit(std::function<void()> task) { queue_.push(std::move(task)); }

    ~ThreadPool() {
        queue_.close();
        // jthread would auto-join, but we join explicitly to guarantee
        // queue is closed and all workers finish before members destruct.
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        workers_.clear();
    }
};
}  // namespace aether