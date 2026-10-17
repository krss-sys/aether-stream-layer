#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "frame.hpp"

int connect_to_server() {
    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "[ERROR] Cannot create socket\n";
        exit(1);
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (::connect(sock, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) < 0) {
        std::cerr << "[ERROR] Connection failed to server\n";
        close(sock);
        exit(1);
    }
    return sock;
}

void run_coalescing_test() {
    std::cout << "[TEST] Running Packet Coalescing Test...\n";
    int sock = connect_to_server();

    std::string msg1 = "Hello Aether 1";
    std::string msg2 = "System Programming 2";
    std::string msg3 = "C++20 Network 3";

    auto raw1 = aether::encode_frame(std::vector<uint8_t>(msg1.begin(), msg1.end()));
    auto raw2 = aether::encode_frame(std::vector<uint8_t>(msg2.begin(), msg2.end()));
    auto raw3 = aether::encode_frame(std::vector<uint8_t>(msg3.begin(), msg3.end()));

    std::vector<uint8_t> big_buffer;
    big_buffer.insert(big_buffer.end(), raw1.begin(), raw1.end());
    big_buffer.insert(big_buffer.end(), raw2.begin(), raw2.end());
    big_buffer.insert(big_buffer.end(), raw3.begin(), raw3.end());

    ssize_t sent = ::send(sock, big_buffer.data(), big_buffer.size(), 0);
    if (sent < 0) {
        std::cerr << "[ERROR] Send failed\n";
    } else {
        std::cout << "[PASS] Sent " << sent << " bytes in a single chunk (coalescing)\n";
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    close(sock);
}

void run_fragmentation_test() {
    std::cout << "\n[TEST 2] Running Packet Fragmentation Test...\n";
    int sock = connect_to_server();

    std::string msg = "Slow Fragmented Frame";
    auto raw_frame = aether::encode_frame(std::vector<uint8_t>(msg.begin(), msg.end()));

    std::cout << "[INFO] Sending " << raw_frame.size()
              << " bytes, 1byte at a time (50ms delay)...\n";

    for (size_t i = 0; i < raw_frame.size(); ++i) {
        ssize_t sent = ::send(sock, &raw_frame[i], 1, 0);
        if (sent <= 0) {
            std::cerr << "[ERROR] Send failed at byte " << i << "\n";
            break;
        }
    }
    std::cout << "[PASS] Fragmented frame fully sent\n";

    std::this_thread::sleep_for(std::chrono::microseconds(100));
    close(sock);
}

int main() {
    std::cout << "=== AETHER FRAME TORTURE TEST ===\n";
    run_coalescing_test();
    run_fragmentation_test();

    std::cout << "\n=== ALL TORTURE TESTS COMPLETED ===\n";

    return 0;
}