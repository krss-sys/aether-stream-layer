#include <dirent.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>

#include "client.hpp"

namespace {

int count_fds() {
    int n = 0;
    for (const auto& entry : std::filesystem::directory_iterator("/proc/self/fd")) {
        if (entry.path().filename().string()[0] != '.') {
            ++n;
        }
    }
    return n;
}

}  // namespace

TEST(AetherClient, ReconnectNoFdLeak) {
    aether::AetherClient c;
    const int before = count_fds();
    constexpr uint16_t kUnreachablePort = 65535;

    for (int i = 0; i < 5; ++i) {
        c.connect("127.0.0.1", kUnreachablePort);
    }

    const int after = count_fds();
    EXPECT_LE(after - before, 1);
}