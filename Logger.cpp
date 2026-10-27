#include "Logger.h"
#include <ctime>
#include <cstdio>

Logger::Logger(const std::string& logFile) : logFile_(logFile) {
    out_.open(logFile_, std::ios::app);
    if (!out_.is_open())
        throw std::runtime_error("Cannot open log file: " + logFile_);
}

void Logger::log(LogLevel level, const std::string& msg) {
    char buf[32];
    std::time_t t = std::time(nullptr);
    std::tm tmv{};
    localtime_r(&t, &tmv);
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmv);
    out_ << buf << " [" << (level == LogLevel::CRITICAL ? "CRITICAL" : "NONCRITICAL")
         << "] " << msg << std::endl;
}
