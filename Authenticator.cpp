#include "Authenticator.h"
#include "sha256.h"
#include <random>
#include <cstdio>
#include <cctype>

uint64_t Authenticator::generateSalt64() {
    std::random_device rd;
    return ((uint64_t)rd() << 32) ^ (uint64_t)rd();
}

std::string Authenticator::formatSalt16(uint64_t salt) const {
    char buf[17];
    std::snprintf(buf, sizeof(buf), "%016llX", (unsigned long long)salt);
    return buf;
}

std::string Authenticator::hashPassword(const std::string& salt, const std::string& password) const {
    return sha256Hex(salt + password);
}

bool Authenticator::verifyHash(const std::string& password, const std::string& salt,
                               const std::string& clientHash) const {
    std::string h = clientHash;
    for (char& c : h) c = (char)std::toupper((unsigned char)c);
    return hashPassword(salt, password) == h;
}
