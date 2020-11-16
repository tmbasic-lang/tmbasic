#include "shared/util/decimal.h"

namespace util {

size_t getDecimalHash(const decimal::Decimal& x) {
    return std::hash<int64_t>{}(x.floor().i64());
}

decimal::Decimal parseDecimalString(const std::string& str) {
    return decimal::Decimal(str);
}

std::string decimalToString(decimal::Decimal x) {
    if (x.isinfinite()) {
        return x < decimal::Decimal(0) ? "-Inf" : "Inf";
    } else if (x.isnan()) {
        return "NaN";
    } else {
        return x.to_eng();
    }
}

}  // namespace util
