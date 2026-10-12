#include "frame.hpp"
#include <cassert>
#include <vector>
#include <string>
#include <iostream>

void test_encode_frame_normal() {
    // 1. Test encoding string "hi"
    // Expectation: First 4 bytes represent length (2 -> 00 00 00 02), followed by 'h' (68) and 'i' (69)
    std::string text = "hi";
    std::vector<uint8_t> payload(text.begin(), text.end());

    std::vector<uint8_t> encoded = encode_frame(payload);

    std::vector<uint8_t> expected = {0x00, 0x00, 0x00, 0x02, 'h', 'i'};
    
    assert(encoded.size() == expected.size());
    for (size_t i = 0; i < encoded.size(); ++i) {
        assert(encoded[i] == expected[i]);
    }
    std::cout << "-> Test encode \"hi\" PASSED!\n";
}

void test_encode_frame_empty() {
    // 2. Test encoding empty payload
    // Expectation: Only 4 zero bytes for length (00 00 00 00)
    std::vector<uint8_t> empty_payload;
    std::vector<uint8_t> encoded = encode_frame(empty_payload);

    std::vector<uint8_t> expected = {0x00, 0x00, 0x00, 0x00};
    
    assert(encoded == expected);
    std::cout << "-> Test encode empty payload PASSED!\n";
}

int main() {
    std::cout << "Running Frame Unit Tests...\n";
    test_encode_frame_normal();
    test_encode_frame_empty();
    std::cout << "ALL FRAME TESTS PASSED SUCCESSFULLY! 🎉\n";
    return 0;
}