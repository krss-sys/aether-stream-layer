#pragma once
#include <string_view>

namespace aether {
class Logger {
   public:
    enum class Level { Debug, Info, Warn, Error };

    static void log(Level level, std::string_view message);
};
}  // namespace aether