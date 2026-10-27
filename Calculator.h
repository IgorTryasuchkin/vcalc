#pragma once
#include <cstdint>
#include <vector>

class Calculator {
public:

    int32_t sumVector(const std::vector<int32_t>& data) const;

    int32_t checkOverflow(int64_t sum) const;
};
