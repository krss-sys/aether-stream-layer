#pragma once
#include <string_view>
#include <cstdint>

namespace aether {
class Logger {
   public:
    enum class Level : uint8_t { Debug, Info, Warn, Error };

    static void log(Level level, std::string_view message);
};
}  // namespace aether