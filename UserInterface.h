#pragma once
#include "ServerConfig.h"
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class UserInterface {
public:
    UserInterface();

    bool parseArgs(int argc, const char** argv);
    ServerConfig getConfig() const { return config_; }

private:
    po::options_description desc_;
    po::variables_map vm_;
    ServerConfig config_;
};
