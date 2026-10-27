#include "ClientBase.h"
#include <fstream>
#include <stdexcept>

void ClientBase::load(const std::string& fileName) {
    std::ifstream in(fileName);
    if (!in.is_open())
        throw std::runtime_error("Cannot open clients database: " + fileName);
    data_.clear();
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        auto pos = line.find(':');
        if (pos == std::string::npos || pos == 0)
            throw std::runtime_error("Bad line in clients database: " + line);
        data_[line.substr(0, pos)] = line.substr(pos + 1);
    }
}

bool ClientBase::checkLogin(const std::string& login) const {
    return data_.find(login) != data_.end();
}

std::string ClientBase::getPasswordByLogin(const std::string& login) const {
    auto it = data_.find(login);
    return it == data_.end() ? std::string() : it->second;
}
