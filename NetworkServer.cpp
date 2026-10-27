#include "NetworkServer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <stdexcept>
#include <string>

void NetworkServer::start() {
    serverSocket_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ < 0)
        throw std::runtime_error(std::string("socket: ") + std::strerror(errno));
    int on = 1;
    ::setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)port_);
    if (::bind(serverSocket_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::string err = std::string("bind: ") + std::strerror(errno);
        close();
        throw std::runtime_error(err);
    }
    if (::listen(serverSocket_, 16) < 0) {
        std::string err = std::string("listen: ") + std::strerror(errno);
        close();
        throw std::runtime_error(err);
    }
}

int NetworkServer::acceptClient() {
    return ::accept(serverSocket_, nullptr, nullptr);
}

void NetworkServer::close() {
    if (serverSocket_ >= 0) {
        ::close(serverSocket_);
        serverSocket_ = -1;
    }
}
