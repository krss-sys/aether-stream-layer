#include "stop_flag.hpp"

namespace aether {
std::atomic<bool> g_stop{false};
}