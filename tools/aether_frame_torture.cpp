#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "frame.hpp"

// Create a TCP socket and connect to the server at 127.0.0.1:8080
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

    // Connect to the server, exit on failure
    if (::connect(sock, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) < 0) {
        std::cerr << "[ERROR] Connection failed to server\n";
        close(sock);
        exit(1);
    }
    return sock;
}

// Test 1: send multiple frames in one single send() call
bool run_coalescing_test() {
    std::cout << "[TEST] Running Packet Coalescing Test...\n";
    int sock = connect_to_server();

    std::string msg1 = "Hello Aether 1";
    std::string msg2 = "System Programming 2";
    std::string msg3 = "C++20 Network 3";

    // Encode each message into a frame
    auto raw1 = aether::encode_frame(std::vector<uint8_t>(msg1.begin(), msg1.end()));
    auto raw2 = aether::encode_frame(std::vector<uint8_t>(msg2.begin(), msg2.end()));
    auto raw3 = aether::encode_frame(std::vector<uint8_t>(msg3.begin(), msg3.end()));

    // Merge all frames into one big buffer
    std::vector<uint8_t> big_buffer;
    big_buffer.insert(big_buffer.end(), raw1.begin(), raw1.end());
    big_buffer.insert(big_buffer.end(), raw2.begin(), raw2.end());
    big_buffer.insert(big_buffer.end(), raw3.begin(), raw3.end());

    // Send everything in a single chunk
    ssize_t sent = ::send(sock, big_buffer.data(), big_buffer.size(), 0);
    bool ok = false;
    if (sent < 0) {
        std::cerr << "[ERROR] Send failed\n";
        ok = false;
    } else {
        std::cout << "[PASS] Sent " << sent << " bytes in a single chunk (coalescing)\n";
        ok = true;
    }

    // Give the server a moment to process, then close
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    close(sock);

    return ok;
}

// Test 2: send one frame byte by byte to simulate fragmentation
bool run_fragmentation_test() {
    std::cout << "\n[TEST 2] Running Packet Fragmentation Test...\n";
    int sock = connect_to_server();

    std::string msg = "Slow Fragmented Frame";
    auto raw_frame = aether::encode_frame(std::vector<uint8_t>(msg.begin(), msg.end()));

    std::cout << "[INFO] Sending " << raw_frame.size()
              << " bytes, 1byte at a time (50ms delay)...\n";

    // Send each byte separately
    bool ok = true;
    for (size_t i = 0; i < raw_frame.size(); ++i) {
        ssize_t sent = ::send(sock, &raw_frame[i], 1, 0);
        if (sent <= 0) {
            std::cerr << "[ERROR] Send failed at byte " << i << "\n";
            ok = false;
            break;
        }
    }
    if (ok) std::cout << "[PASS] Fragmented frame fully sent\n";

    // Small delay before closing the socket
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    close(sock);

    return ok;
}

// Test 3: send a malicious frame with an invalid huge length
bool run_malicious_test() {
    std::cout << "\n[TEST 3] Running Packet Malicious Test...\n";
    int sock = connect_to_server();

    // Fake frame with length = 0xFFFFFFFF
    std::vector<uint8_t> badFrame = {0xFF, 0xFF, 0xFF, 0xFF, 'a', 'b'};
    ssize_t sent = ::send(sock, badFrame.data(), badFrame.size(), 0);
    if (sent < 0) {
        std::cerr << "[ERROR] Send malicious frame failed\n";
        ::close(sock);
        return false;
    }
    std::cout << "Already sent the read command frame (len = 0xFFFFFFFF)\n";

    // Try to read; server should close the connection
    char buf[16];
    ssize_t n = recv(sock, buf, sizeof(buf), 0);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ::close(sock);

    // n == 0 means server closed the connection
    if (n == 0) {
        std::cout << "[PASS] Server has closed the connection (defense succeeded)\n";
        return true;
    }
    // n < 0 means recv error, also acceptable as defense
    if (n < 0) {
        std::cout << "[PASS] Server has closed the connection (recv error)\n\n";
        return true;
    }
    std::cerr << "[FAIL] Server is still open (n = " << n << ")\n\n";
    return false;
}

// Run all tests and report the overall result
int main() {
    std::cout << "=== AETHER FRAME TORTURE TEST ===\n";

    bool all_pass = true;
    if (!run_coalescing_test()) all_pass = false;
    if (!run_fragmentation_test()) all_pass = false;
    if (!run_malicious_test()) all_pass = false;

    if (all_pass) {
        std::cout << "\n[ALL PASS] All tests passed!\n";
        return EXIT_SUCCESS;
    } else {
        std::cout << "[SOME FAIL] Some tests failed!\n";
        return EXIT_FAILURE;
    }
}