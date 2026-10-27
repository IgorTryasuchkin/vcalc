#pragma once
#include <string>

class ServerConfig {
public:
    ServerConfig(std::string baseFile = "/etc/vcalc.conf",
                 std::string logFile = "/var/log/vcalc.log",
                 int port = 33333)
        : baseFile_(std::move(baseFile)), logFile_(std::move(logFile)), port_(port) {}

    std::string getBaseFile() const { return baseFile_; }
    std::string getLogFile() const { return logFile_; }
    int getPort() const { return port_; }

private:
    std::string baseFile_;
    std::string logFile_;
    int port_;
};
