#include "DataSerializer.h"
#include <cstring>

std::vector<uint8_t> DataSerializer::serializeU32(uint32_t v) {
    std::vector<uint8_t> b(sizeof(v));
    std::memcpy(b.data(), &v, sizeof(v));
    return b;
}

uint32_t DataSerializer::deserializeU32(const std::vector<uint8_t>& bytes) {
    uint32_t v = 0;
    if (bytes.size() >= sizeof(v)) std::memcpy(&v, bytes.data(), sizeof(v));
    return v;
}

std::vector<int32_t> DataSerializer::deserializeVector(const std::vector<uint8_t>& bytes) {
    std::vector<int32_t> r(bytes.size() / sizeof(int32_t));
    if (!r.empty()) std::memcpy(r.data(), bytes.data(), r.size() * sizeof(int32_t));
    return r;
}

std::vector<uint8_t> DataSerializer::serializeResults(const std::vector<int32_t>& results) {
    std::vector<uint8_t> b(sizeof(uint32_t) + results.size() * sizeof(int32_t));
    uint32_t n = (uint32_t)results.size();
    std::memcpy(b.data(), &n, sizeof(n));
    if (!results.empty())
        std::memcpy(b.data() + sizeof(n), results.data(), results.size() * sizeof(int32_t));
    return b;
}

std::vector<uint8_t> DataSerializer::serializeInt32(int32_t v) {
    std::vector<uint8_t> b(sizeof(v));
    std::memcpy(b.data(), &v, sizeof(v));
    return b;
}
