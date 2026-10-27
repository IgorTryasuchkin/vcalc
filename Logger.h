#pragma once
#include <string>
#include <fstream>
#include <stdexcept>

enum class LogLevel { NONCRITICAL, CRITICAL };

class Logger {
public:

    explicit Logger(const std::string& logFile);

    void log(LogLevel level, const std::string& msg);
    void logCritical(const std::string& msg) { log(LogLevel::CRITICAL, msg); }
    void logNonCritical(const std::string& msg) { log(LogLevel::NONCRITICAL, msg); }

private:
    std::string logFile_;
    std::ofstream out_;
};
