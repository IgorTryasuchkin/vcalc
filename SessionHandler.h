#pragma once
#include "ClientBase.h"
#include "Authenticator.h"
#include "Calculator.h"
#include "Logger.h"
#include <cstdint>
#include <string>
#include <vector>

class SessionHandler {
public:
    SessionHandler(int clientSocket, const ClientBase& base, Authenticator& auth,
                   const Calculator& calc, Logger& logger);
    ~SessionHandler();
    SessionHandler(const SessionHandler&) = delete;
    SessionHandler& operator=(const SessionHandler&) = delete;

    void handleSession();

    static constexpr uint32_t MAX_VECTOR_SIZE = 1u << 26;

    static constexpr int RECV_TIMEOUT_SEC = 30;

private:
    bool handleAuthentication();
    void receiveAndProcessVectors();

    void sendBytes(const void* data, size_t len);
    void recvExact(void* data, size_t len);
    std::string recvText(size_t maxLen);
    void sendText(const std::string& s) { sendBytes(s.data(), s.size()); }
    void sendError() ;

    int sock_;
    const ClientBase& base_;
    Authenticator& auth_;
    const Calculator& calc_;
    Logger& logger_;
};
