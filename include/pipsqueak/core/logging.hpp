//
// Created by Daftpy on 7/26/2025.
//

#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <iostream>
#include <string>
#include <mutex>
#include <chrono>
#include <iomanip>

namespace pipsqueak::core::logging {
    class Logger {
    public:
        static void log(const std::string& tag, const std::string& message) {
            std::lock_guard<std::mutex> lock(mutex_);

            const auto now = std::chrono::system_clock::now();
            const auto timeT = std::chrono::system_clock::to_time_t(now);

            // Thread-safe versions of localtime
            tm tm{};

            #ifdef _WIN32
                localtime_s(&tm, &timeT);
            #else
                localtime_r(&timeT, &tm);
            #endif

            std::cout << "[" << std::put_time(&tm, "%T") << "] " // HH:MM:SS
                << "[" << tag << "] "
                << message << std::endl;
        }

    private:
        static inline std::mutex mutex_;
    };

}

#endif //LOGGING_HPP
