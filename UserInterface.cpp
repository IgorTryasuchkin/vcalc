#include "UserInterface.h"
#include <iostream>

UserInterface::UserInterface() : desc_("Usage: vcalc [options]\nOptions") {
    desc_.add_options()
        ("help,h", "show this help")
        ("base,b", po::value<std::string>()->default_value("/etc/vcalc.conf"),
         "clients database file (login:password per line)")
        ("log,l", po::value<std::string>()->default_value("/var/log/vcalc.log"),
         "log file")
        ("port,p", po::value<int>()->default_value(33333), "TCP port (1-65535)");
}

bool UserInterface::parseArgs(int argc, const char** argv) {
    if (argc < 2) {
        std::cout << desc_ << std::endl;
        return false;
    }
    try {
        po::store(po::parse_command_line(argc, argv, desc_), vm_);
        po::notify(vm_);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n" << desc_ << std::endl;
        return false;
    }
    if (vm_.count("help")) {
        std::cout << desc_ << std::endl;
        return false;
    }
    int port = vm_["port"].as<int>();
    if (port < 1 || port > 65535) {
        std::cerr << "Error: port must be in range 1-65535\n";
        return false;
    }
    config_ = ServerConfig(vm_["base"].as<std::string>(), vm_["log"].as<std::string>(), port);
    return true;
}
