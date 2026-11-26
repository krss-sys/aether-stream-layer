#include <gtest/gtest.h>

#include <random>
#include <vector>

#include "message.hpp"

using namespace aether;

TEST(MessageCodec, RoundTripProduce) {
    Request req{Command::kProduce, "topic1", {'h', 'e', 'l', 'l', 'o'}};
    auto bytes = encode_request(req);
    auto decoded = decode_request(bytes);

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->cmd, Command::kProduce);
    EXPECT_EQ(decoded->topic, "topic1");
    EXPECT_EQ(decoded->body, std::vector<uint8_t>({'h', 'e', 'l', 'l', 'o'}));
}

TEST(MessageCodec, RoundTripPing) {
    Request req{Command::kPing, "", {}};
    auto bytes = encode_request(req);
    auto decoded = decode_request(bytes);

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->cmd, Command::kPing);
    EXPECT_TRUE(decoded->topic.empty());
    EXPECT_TRUE(decoded->body.empty());
}

TEST(MessageCodec, RoundTripFetch) {
    Request req{Command::kFetch, "t", {0x00, 0x03, 'a', 'b', 'c', 0x00, 0x00, 0x00, 0x0A}};
    auto bytes = encode_request(req);
    auto decoded = decode_request(bytes);

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->cmd, Command::kFetch);
    EXPECT_EQ(decoded->topic, "t");
    EXPECT_EQ(decoded->body,
              std::vector<uint8_t>({0x00, 0x03, 'a', 'b', 'c', 0x00, 0x00, 0x00, 0x0A}));
}

TEST(MessageCodec, RoundTripResponse) {
    std::vector<uint8_t> body = {0x01, 0x02, 0x03};
    auto bytes = encode_response(StatusCode::kOk, body);
    auto decoded = decode_response(bytes);

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->status, StatusCode::kOk);
    EXPECT_EQ(decoded->body, body);
}

TEST(MessageCodec, HexPingRequest) {
    std::vector<uint8_t> payload = {0x04, 0x00, 0x00};
    auto decoded = decode_request(payload);

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->cmd, Command::kPing);
    EXPECT_TRUE(decoded->topic.empty());
    EXPECT_TRUE(decoded->body.empty());
}

TEST(MessageCodec, HexProduceRequest) {
    std::vector<uint8_t> payload = {0x01, 0x00, 0x01, 0x74, 0x68, 0x69};
    auto decoded = decode_request(payload);

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->cmd, Command::kProduce);
    EXPECT_EQ(decoded->topic, "t");
    EXPECT_EQ(decoded->body, std::vector<uint8_t>({'h', 'i'}));
}

TEST(MessageCodec, TruncatedHeaderAlwaysNullopt) {
    std::vector<uint8_t> full =
        encode_request(Request{Command::kProduce, "topic1", {'h', 'e', 'l', 'l', 'o'}});

    for (size_t cut = 0; cut < 9; ++cut) {
        std::vector<uint8_t> truncated(full.begin(),
                                       full.begin() + static_cast<std::ptrdiff_t>(cut));
        auto decoded = decode_request(truncated);
        EXPECT_FALSE(decoded.has_value()) << "expected nullopt at cut = " << cut;
    }
}

TEST(MessageCodec, TruncatedProduceBodyIsStillValid) {
    std::vector<uint8_t> full =
        encode_request(Request{Command::kProduce, "t", {'h', 'e', 'l', 'l', 'o'}});
    std::vector<uint8_t> cut_body(full.begin(), full.begin() + 6);
    auto decoded = decode_request(cut_body);

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->cmd, Command::kProduce);
    EXPECT_EQ(decoded->topic, "t");
    EXPECT_EQ(decoded->body, std::vector<uint8_t>({'h', 'e'}));
}

TEST(MessageCodec, TopicLengthTooLarge) {
    std::vector<uint8_t> payload = {0x01, 0x00, 0x64, 'a', 'b'};
    auto decoded = decode_request(payload);
    EXPECT_FALSE(decoded.has_value());
}

TEST(MessageCodec, EmptyTopicRejectedForNonPing) {
    std::vector<uint8_t> payload = {0x01, 0x00, 0x00};
    auto decoded = decode_request(payload);
    EXPECT_FALSE(decoded.has_value());
}

TEST(MessageCodec, ForbiddenCharsInTopic) {
    std::vector<uint8_t> payload = {0x01, 0x00, 0x03, 'a', '/', 'b'};
    auto decoded = decode_request(payload);
    EXPECT_FALSE(decoded.has_value());
}

TEST(MessageCodec, TopicTooLong) {
    std::string long_topic(65, 'a');
    Request req{Command::kProduce, long_topic, {}};
    auto bytes = encode_request(req);
    auto decoded = decode_request(bytes);
    EXPECT_FALSE(decoded.has_value());
}

TEST(MessageCodec, FuzzLiteNoCrash) {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> len_dist(0, 64);
    std::uniform_int_distribution<int> byte_dist(0, 255);

    for (int i = 0; i < 100000; ++i) {
        int len = len_dist(rng);
        std::vector<uint8_t> random_bytes(len);
        for (auto& b : random_bytes) {
            b = static_cast<uint8_t>(byte_dist(rng));
        }
        (void)decode_request(random_bytes);
    }
    SUCCEED();
}