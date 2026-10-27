#pragma once
#include <cstdint>
#include <vector>

class DataSerializer {
public:
    static std::vector<uint8_t> serializeU32(uint32_t v);
    static uint32_t deserializeU32(const std::vector<uint8_t>& bytes);

    static std::vector<int32_t> deserializeVector(const std::vector<uint8_t>& bytes);

    static std::vector<uint8_t> serializeInt32(int32_t v);
    static std::vector<uint8_t> serializeResults(const std::vector<int32_t>& results);
};
