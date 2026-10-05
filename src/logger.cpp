#include "logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

namespace aether {
// Global mutex to syschronize log output across threads
static std::mutex g_log_mutex;

void Logger::log(Level level, std::string_view message) {
    // --- A. GET CURRENT TIME (system_clock + duration_cast) ---
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);

    // Extract fractional milliseconds
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // Thread_safe conversion to local time using localtime_r
    struct tm time_info;
    localtime_r(&time_t_now, &time_info);

    // --- B. CONSTRUCT LOG STRING IN MEMORY (ostringstream) ---
    std::ostringstream ss;

    // 1. Format Time: [HH:MM:SS.mmm]
    ss << "[" << std::put_time(&time_info, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3)
       << ms.count() << "] ";

    // 2. Format Severity Level
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

    // 3. Format Thread ID
    ss << "[" << std::this_thread::get_id() << "] ";

    // 4. Log Message
    ss << message << "\n";

    // --- C. LOCK MUTEX & FLUSH TO CERR (THREAD-SAFE) ---
    std::lock_guard<std::mutex> lock(g_log_mutex);
    std::cerr << ss.str();
}
}  // namespace aether