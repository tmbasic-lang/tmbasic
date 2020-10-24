#include "decimal.h"

using namespace util;

size_t util::getDecimalHash(const decimal::Decimal& x) {
    return std::hash<int64_t>{}(x.floor().i64());
}

decimal::Decimal util::parseDecimalString(const std::string& str) {
    return decimal::Decimal(str);
}

std::string util::decimalToString(decimal::Decimal x) {
    if (x.isinfinite()) {
        return x < decimal::Decimal(0) ? "-Inf" : "Inf";
    } else if (x.isnan()) {
        return "NaN";
    } else {
        return x.to_eng();
    }
}
