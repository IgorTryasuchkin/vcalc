#include "Calculator.h"
#include <limits>

int32_t Calculator::checkOverflow(int64_t sum) const {
    if (sum > std::numeric_limits<int32_t>::max()) return std::numeric_limits<int32_t>::max();
    if (sum < std::numeric_limits<int32_t>::min()) return std::numeric_limits<int32_t>::min();
    return (int32_t)sum;
}

int32_t Calculator::sumVector(const std::vector<int32_t>& data) const {

    int64_t sum = 0;
    for (int32_t v : data) sum += v;
    return checkOverflow(sum);
}
