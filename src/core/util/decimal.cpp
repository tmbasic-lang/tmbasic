#include "decimal.h"

using namespace util;

std::decimal::decimal64 util::parseDecimalString(const std::string& str) {
    auto lowercase = boost::algorithm::to_lower_copy(str);
    if (lowercase == "inf") {
        std::decimal::decimal64 inf = 1;
        inf /= 0;
        return inf;
    } else if (lowercase == "-inf") {
        std::decimal::decimal64 inf = -1;
        inf /= 0;
        return inf;
    } else if (lowercase == "nan") {
        std::decimal::decimal64 nan = 0;
        nan /= 0;
        return nan;
    }

    std::decimal::decimal64 sign = 1;
    std::decimal::decimal64 integerPart = 0;
    std::decimal::decimal64 decimalDividend = 0;
    std::decimal::decimal64 decimalDivisor = 1;
    auto isFirstChar = true;
    auto inIntegerPart = true;

    for (auto ch : str) {
        if (isFirstChar && ch == '-') {
            sign = -1;
            isFirstChar = false;
            continue;
        }
        isFirstChar = false;

        if (inIntegerPart) {
            if (ch == '.') {
                inIntegerPart = false;
            } else {
                integerPart *= 10;
                integerPart += ch - '0';
            }
        } else {
            decimalDividend *= 10;
            decimalDividend += ch - '0';
            decimalDivisor *= 10;
        }
    }

    return sign * (integerPart + decimalDividend / decimalDivisor);
}

std::string util::decimalToString(std::decimal::decimal64 x) {
    auto dbl = std::decimal::decimal64_to_double(x);
    if (__builtin_isinf(dbl)) {
        return dbl < 0 ? "-Inf" : "Inf";
    } else if (__builtin_isnan(dbl)) {
        return "NaN";
    }

    std::ostringstream ss;
    if (x < 0) {
        ss << '-';
        x *= -1;
    }
    auto integerPart = std::decimal::decimal64_to_long_long(x);
    ss << integerPart;
    x -= integerPart;
    if (x != 0) {
        ss << '.';
    }
    while (x != 0) {
        x *= 10;
        auto digit = std::decimal::decimal64_to_long_long(x);
        ss << digit;
        x -= digit;
    }
    return ss.str();
}
