#include <gtest/gtest.h>
#include "client.hpp"
#include <dirent.h>
#include <string>

static int count_fds() {
    int n = 0;
    DIR* d = opendir("/proc/self/fd");
    while (auto* e = readdir(d)) if (e->d_name[0] != '.') ++n;
    closedir(d);
    return n;
}

TEST(AetherClient, ReconnectNoFdLeak) {
    aether::AetherClient c;
    int before = count_fds();
    for (int i = 0; i < 5; ++i) {
        c.connect("127.0.0.1", 65535);  // sẽ fail, không sao
    }
    int after = count_fds();
    EXPECT_LE(after - before, 1);  // cho phép chênh 1 fd (test infra)
}