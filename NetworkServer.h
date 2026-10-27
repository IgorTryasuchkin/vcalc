#pragma once

class NetworkServer {
public:
    explicit NetworkServer(int port) : port_(port) {}
    ~NetworkServer() { close(); }
    NetworkServer(const NetworkServer&) = delete;
    NetworkServer& operator=(const NetworkServer&) = delete;

    void start();

    int acceptClient();
    void close();

private:
    int port_;
    int serverSocket_ = -1;
};
