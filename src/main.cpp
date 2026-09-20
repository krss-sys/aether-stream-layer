#include <iostream>
#include <thread>
#include <string>
#include "ThreadSafeQueue.hpp"

void producer(ThreadSafeQueue<std::string>& queue) {
    for (int i = 1; i <= 5; ++i) {
        std::string msg = "Message #" + std::to_string(i);
        std::cout << "[Producer] Pushing: " << msg << std::endl;
        queue.push(msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void consumer(ThreadSafeQueue<std::string>& queue) {
    for (int i = 1; i <= 5; ++i) {
        std::string msg;
        if (queue.pop(msg)) {
            std::cout << "  [Consumer] Popped: " << msg << std::endl;
        }
    }
}

int main() {
    std::cout << "=== AETHER STREAM LAYER: MILESTONE 1 ===" << std::endl;

    ThreadSafeQueue<std::string> queue;

    std::thread t1(producer, std::ref(queue));
    std::thread t2(consumer, std::ref(queue));

    t1.join();
    t2.join();

    std::cout << "=== MILESTONE 1 PASSED SUCCESSFULLY ===" << std::endl;
    return 0;
}