#pragma once
#include <map>
#include <string>

class ClientBase {
public:

    void load(const std::string& fileName);

    bool checkLogin(const std::string& login) const;

    std::string getPasswordByLogin(const std::string& login) const;
    std::size_t size() const { return data_.size(); }

private:
    std::map<std::string, std::string> data_;
};
