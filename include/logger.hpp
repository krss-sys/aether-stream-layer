#ifndef AETHER_LOGGER_HPP
#define AETHER_LOGGER_HPP
#include <string_view>

namespace aether {
class Logger {
   public:
    enum class Level { Debug, Info, Warn, Error };

    static void log(Level level, std::string_view message);
};
}  // namespace aether
#endif // AETHER_LOGGER_HPP