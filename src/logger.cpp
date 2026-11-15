#include "logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

namespace aether {
static std::mutex g_log_mutex;

void Logger::log(Level level, std::string_view message) {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    struct tm time_info;
    localtime_r(&time_t_now, &time_info);

    std::ostringstream ss;

    ss << "[" << std::put_time(&time_info, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3)
       << ms.count() << "] ";

    switch (level) {
        case Level::Debug:
            ss << "[DEBUG] ";
            break;
        case Level::Info:
            ss << "[INFO] ";
            break;
        case Level::Warn:
            ss << "[WARN] ";
            break;
        case Level::Error:
            ss << "[ERROR] ";
            break;
    }

    ss << "[" << std::this_thread::get_id() << "] ";

    ss << message << "\n";

    std::lock_guard<std::mutex> lock(g_log_mutex);
    std::cerr << ss.str();
}
}  // namespace aether