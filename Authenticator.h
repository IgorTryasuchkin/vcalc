#pragma once
#include <cstdint>
#include <string>

class Authenticator {
public:

    uint64_t generateSalt64();

    std::string formatSalt16(uint64_t salt) const;

    std::string hashPassword(const std::string& salt, const std::string& password) const;

    bool verifyHash(const std::string& password, const std::string& salt,
                    const std::string& clientHash) const;
};
