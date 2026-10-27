#include "SessionHandler.h"
#include "DataSerializer.h"
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>

SessionHandler::SessionHandler(int clientSocket, const ClientBase& base, Authenticator& auth,
                               const Calculator& calc, Logger& logger)
    : sock_(clientSocket), base_(base), auth_(auth), calc_(calc), logger_(logger) {
    timeval tv{RECV_TIMEOUT_SEC, 0};
    ::setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    ::setsockopt(sock_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

SessionHandler::~SessionHandler() {
    if (sock_ >= 0) ::close(sock_);
}

void SessionHandler::sendBytes(const void* data, size_t len) {
    const char* p = static_cast<const char*>(data);
    while (len > 0) {
        ssize_t n = ::send(sock_, p, len, MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error(std::string("send: ") + std::strerror(errno));
        }
        p += n;
        len -= (size_t)n;
    }
}

void SessionHandler::recvExact(void* data, size_t len) {
    char* p = static_cast<char*>(data);
    while (len > 0) {
        ssize_t n = ::recv(sock_, p, len, 0);
        if (n == 0) throw std::runtime_error("client closed connection");
        if (n < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error(std::string("recv: ") + std::strerror(errno));
        }
        p += n;
        len -= (size_t)n;
    }
}

std::string SessionHandler::recvText(size_t maxLen) {
    std::string buf(maxLen, '\0');
    ssize_t n;
    do { n = ::recv(sock_, &buf[0], maxLen, 0); } while (n < 0 && errno == EINTR);
    if (n == 0) throw std::runtime_error("client closed connection");
    if (n < 0) throw std::runtime_error(std::string("recv: ") + std::strerror(errno));
    buf.resize((size_t)n);
    while (!buf.empty() && (buf.back() == '\n' || buf.back() == '\r')) buf.pop_back();
    return buf;
}

void SessionHandler::sendError() {
    try { sendText("ERR"); } catch (...) {  }
}

bool SessionHandler::handleAuthentication() {
    std::string login = recvText(512);
    const size_t SALT_LEN = 16, HASH_LEN = 64;
    if (login.size() > SALT_LEN + HASH_LEN) {
        std::string hash = login.substr(login.size() - HASH_LEN);
        std::string salt16 = login.substr(login.size() - HASH_LEN - SALT_LEN, SALT_LEN);
        login.resize(login.size() - HASH_LEN - SALT_LEN);
        if (!base_.checkLogin(login)) {
            logger_.logNonCritical("Authentication: unknown login '" + login + "'");
            sendError();
            return false;
        }
        if (!auth_.verifyHash(base_.getPasswordByLogin(login), salt16, hash)) {
            logger_.logNonCritical("Authentication: wrong password for login '" + login + "'");
            sendError();
            return false;
        }
        sendText("OK");
        return true;
    }
    if (!base_.checkLogin(login)) {
        logger_.logNonCritical("Authentication: unknown login '" + login + "'");
        sendError();
        return false;
    }
    std::string salt16 = auth_.formatSalt16(auth_.generateSalt64());
    sendText(salt16);

    std::string hash(64, '\0');
    recvExact(&hash[0], hash.size());
    if (!auth_.verifyHash(base_.getPasswordByLogin(login), salt16, hash)) {
        logger_.logNonCritical("Authentication: wrong password for login '" + login + "'");
        sendError();
        return false;
    }
    sendText("OK");
    return true;
}

void SessionHandler::receiveAndProcessVectors() {
    std::vector<uint8_t> u32(4);
    recvExact(u32.data(), 4);
    uint32_t count = DataSerializer::deserializeU32(u32);

    for (uint32_t i = 0; i < count; ++i) {
        recvExact(u32.data(), 4);
        uint32_t size = DataSerializer::deserializeU32(u32);
        if (size > MAX_VECTOR_SIZE) {
            logger_.logNonCritical("Vector #" + std::to_string(i) + " too large: " +
                                   std::to_string(size));
            return;
        }
        std::vector<uint8_t> raw((size_t)size * 4);
        if (!raw.empty()) recvExact(raw.data(), raw.size());
        int32_t result = calc_.sumVector(DataSerializer::deserializeVector(raw));
        auto out = DataSerializer::serializeInt32(result);
        sendBytes(out.data(), out.size());
    }
}

void SessionHandler::handleSession() {
    try {
        if (!handleAuthentication()) return;
        receiveAndProcessVectors();

    } catch (const std::exception& e) {
        logger_.logNonCritical(std::string("Session error: ") + e.what());
    }
}
