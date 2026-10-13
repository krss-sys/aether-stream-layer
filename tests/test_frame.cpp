#include <gtest/gtest.h>
#include "frame.hpp"
#include <vector>
#include <string>

// ==========================================
// 1. TEST SUITE CHO ENCODE FRAME
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
// 2. TEST SUITE CHO DECODER (FRAME DECODER)
// ==========================================
TEST(FrameDecoderTest, SingleCompleteFrame) {
    aether::FrameDecoder decoder;
    std::vector<uint8_t> payload = {'h', 'e', 'l', 'l', 'o'};
    auto encoded = aether::encode_frame(payload);

    decoder.feed(encoded.data(), encoded.size());

    auto result = decoder.next();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), payload);
    EXPECT_FALSE(decoder.next().has_value()); // Không còn frame nào nữa
}

TEST(FrameDecoderTest, StickyPacketsMultipleFrames) {
    aether::FrameDecoder decoder;

    std::vector<uint8_t> p1 = {'a', 'b', 'c'};
    std::vector<uint8_t> p2 = {'d', 'e'};
    std::vector<uint8_t> p3 = {'f', 'g', 'h', 'i'};

    auto e1 = aether::encode_frame(p1);
    auto e2 = aether::encode_frame(p2);
    auto e3 = aether::encode_frame(p3);

    // Ghép dính các gói tin lại với nhau (Sticky Packets)
    std::vector<uint8_t> stream;
    stream.insert(stream.end(), e1.begin(), e1.end());
    stream.insert(stream.end(), e2.begin(), e2.end());
    stream.insert(stream.end(), e3.begin(), e3.end());

    // Feed toàn bộ vào 1 lần duy nhất
    decoder.feed(stream.data(), stream.size());

    // Kiểm tra từng gói được tách ra chuẩn xác
    auto r1 = decoder.next();
    ASSERT_TRUE(r1.has_value());
    EXPECT_EQ(r1.value(), p1);

    auto r2 = decoder.next();
    ASSERT_TRUE(r2.has_value());
    EXPECT_EQ(r2.value(), p2);

    auto r3 = decoder.next();
    ASSERT_TRUE(r3.has_value());
    EXPECT_EQ(r3.value(), p3);

    EXPECT_FALSE(decoder.next().has_value()); // Buffer đã sạch sẽ
}