#pragma once
#include <atomic>

namespace aether {
inline std::atomic<bool> g_stop{false};
}