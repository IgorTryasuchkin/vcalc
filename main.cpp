#include "UserInterface.h"
#include "Logger.h"
#include "ClientBase.h"
#include "Authenticator.h"
#include "Calculator.h"
#include "NetworkServer.h"
#include "SessionHandler.h"
#include <csignal>
#include <cerrno>
#include <iostream>
#include <memory>

static volatile sig_atomic_t g_running = 1;
static void onSignal(int) { g_running = 0; }

int main(int argc, const char** argv) {
    UserInterface ui;
    if (!ui.parseArgs(argc, argv)) return argc < 2 ? 0 : 1;
    ServerConfig cfg = ui.getConfig();

    std::unique_ptr<Logger> logger;
    try {
        logger = std::make_unique<Logger>(cfg.getLogFile());
    } catch (const std::exception& e) {
        std::cerr << "Critical: " << e.what() << std::endl;
        return 2;
    }

    ClientBase base;
    try {
        base.load(cfg.getBaseFile());
    } catch (const std::exception& e) {
        logger->logCritical(e.what());
        return 3;
    }

    NetworkServer server(cfg.getPort());
    try {
        server.start();
    } catch (const std::exception& e) {
        logger->logCritical(e.what());
        return 4;
    }

    struct sigaction sa{};
    sa.sa_handler = onSignal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    Authenticator auth;
    Calculator calc;

    while (g_running) {
        int client = server.acceptClient();
        if (client < 0) {
            if (errno != EINTR) logger->logNonCritical("accept failed, errno=" + std::to_string(errno));
            continue;
        }
        SessionHandler session(client, base, auth, calc, *logger);
        session.handleSession();
    }
    server.close();
    return 0;
}
