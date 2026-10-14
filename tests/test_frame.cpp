#include <arpa/inet.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "frame.hpp"

// ==========================================
// 1. TEST SUITE FOR ENCODE FRAME
// ==========================================
TEST(FrameTest, EncodeNormalPayload) {
    std::string text = "hi";
    std::vector<uint8_t> payload(text.begin(), text.end());

    std::vector<uint8_t> encoded = aether::encode_frame(payload);
    std::vector<uint8_t> expected = {0x00, 0x00, 0x00, 0x02, 'h', 'i'};

    ASSERT_EQ(encoded.size(), expected.size());
    for (size_t i = 0; i < encoded.size(); ++i) {
        EXPECT_EQ(encoded[i], expected[i]);
    }
}

TEST(FrameTest, EncodeEmptyPayload) {
    std::vector<uint8_t> empty_payload;
    std::vector<uint8_t> encoded = aether::encode_frame(empty_payload);
    std::vector<uint8_t> expected = {0x00, 0x00, 0x00, 0x00};

    EXPECT_EQ(encoded, expected);
}

// ==========================================
// 2. TEST SUITE FOR DECODER (FRAME DECODER)
// ==========================================
TEST(FrameDecoderTest, SingleCompleteFrame) {
    aether::FrameDecoder decoder;
    std::vector<uint8_t> payload = {'h', 'e', 'l', 'l', 'o'};
    auto encoded = aether::encode_frame(payload);

    decoder.feed(encoded.data(), encoded.size());

    auto result = decoder.next();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), payload);
    EXPECT_FALSE(decoder.next().has_value());  // No more frames left
}

TEST(FrameDecoderTest, StickyPacketsMultipleFrames) {
    aether::FrameDecoder decoder;

    std::vector<uint8_t> p1 = {'a', 'b', 'c'};
    std::vector<uint8_t> p2 = {'d', 'e'};
    std::vector<uint8_t> p3 = {'f', 'g', 'h', 'i'};

    auto e1 = aether::encode_frame(p1);
    auto e2 = aether::encode_frame(p2);
    auto e3 = aether::encode_frame(p3);

    // Concatenate the packets together (Sticky Packets)
    std::vector<uint8_t> stream;
    stream.insert(stream.end(), e1.begin(), e1.end());
    stream.insert(stream.end(), e2.begin(), e2.end());
    stream.insert(stream.end(), e3.begin(), e3.end());

    // Feed everything in a single call
    decoder.feed(stream.data(), stream.size());

    // Verify each packet is split out correctly
    auto r1 = decoder.next();
    ASSERT_TRUE(r1.has_value());
    EXPECT_EQ(r1.value(), p1);

    auto r2 = decoder.next();
    ASSERT_TRUE(r2.has_value());
    EXPECT_EQ(r2.value(), p2);

    auto r3 = decoder.next();
    ASSERT_TRUE(r3.has_value());
    EXPECT_EQ(r3.value(), p3);

    EXPECT_FALSE(decoder.next().has_value());  // Buffer is now clean
}

TEST(FrameDecoderTest, FeedByteByByte) {
    aether::FrameDecoder decoder;
    std::vector<uint8_t> payload = {'h', 'e', 'l', 'l', 'o'};
    auto encoded = aether::encode_frame(payload);  // Standard packet: 4-byte length + payload

    // Feed the decoder one byte at a time
    for (size_t i = 0; i < encoded.size() - 1; ++i) {
        decoder.feed(&encoded[i], 1);
        EXPECT_FALSE(decoder.next().has_value());  // Not enough data yet, so no frame is released
    }

    // Feed the last byte
    decoder.feed(&encoded.back(), 1);

    // Now a complete frame must be extracted
    auto result = decoder.next();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), payload);
}

TEST(FrameDecoderTest, DosMitigationAndEmptyPayload) {
    // Case 1: Send a huge len (0xFFFFFFFF) exceeding the kMaxFrameSize threshold
    {
        aether::FrameDecoder decoder;
        uint32_t malicious_len = htonl(0xFFFFFFFF);

        // Feed in the 4 malicious length bytes
        decoder.feed(reinterpret_cast<const uint8_t*>(&malicious_len), 4);

        // Call next() to trigger the check logic
        auto res = decoder.next();

        EXPECT_FALSE(res.has_value());
        EXPECT_TRUE(decoder.has_error());  // The DoS error flag must be set immediately!
    }

    // Case 2: Empty payload (len = 0) is perfectly valid
    {
        aether::FrameDecoder decoder;
        std::vector<uint8_t> empty_payload = {};
        auto encoded = aether::encode_frame(empty_payload);

        decoder.feed(encoded.data(), encoded.size());

        auto res = decoder.next();
        ASSERT_TRUE(res.has_value());
        EXPECT_TRUE(res.value().empty());
        EXPECT_FALSE(decoder.has_error());
    }
}