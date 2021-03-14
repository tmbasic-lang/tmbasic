#include "util/decimal.h"

using decimal::Decimal;

namespace util {

size_t getDecimalHash(const Decimal& x) {
    return std::hash<int64_t>{}(x.floor().i64());
}

Decimal parseDecimalString(const std::string& str) {
    return Decimal(str);
}

std::string decimalToString(const Decimal& x) {
    if (x.isinfinite()) {
        return x < Decimal(0) ? "-Inf" : "Inf";
    }
    if (x.isnan()) {
        return "NaN";
    }

    auto str = x.format("f");
    auto len = str.size();

    if (str.find('.') != std::string::npos) {
        // trim trailing zeroes after the decimal point
        while (str[len - 1] == '0') {
            len--;
        }
        if (str[len - 1] == '.') {
            len--;
        }
    }

    return len == str.size() ? str : str.substr(0, len);
}

Decimal doubleToDecimal(double x) {
    if (std::isnan(x)) {
        mpd_uint128_triple_t nanTriple;
        memset(&nanTriple, 0, sizeof(nanTriple));
        nanTriple.tag = MPD_TRIPLE_QNAN;
        return Decimal(nanTriple);
    }

    ieee754_double decomposed{};
    decomposed.d = x;

    if (std::isinf(x)) {
        mpd_uint128_triple_t infTriple;
        memset(&infTriple, 0, sizeof(infTriple));
        infTriple.tag = MPD_TRIPLE_INF;
        infTriple.sign = decomposed.ieee.negative;
        return Decimal(infTriple);
    }

    int64_t binaryExponent = decomposed.ieee.exponent;
    binaryExponent -= IEEE754_DOUBLE_BIAS;

    uint64_t mantissa = decomposed.ieee.mantissa0;
    mantissa <<= 32;
    mantissa |= decomposed.ieee.mantissa1;

    Decimal fractionNumerator = mantissa;

    Decimal fractionDenominator = 2;
    fractionDenominator = fractionDenominator.pow(52);

    Decimal fraction = fractionNumerator / fractionDenominator;

    Decimal magnitude = 2;
    magnitude = magnitude.pow(binaryExponent);

    Decimal sign = decomposed.ieee.negative == 0U ? 1 : -1;

    if (mantissa == 0 && decomposed.ieee.exponent == 0U) {
        return sign * fraction * magnitude;
    }
    return sign * (fraction + 1) * magnitude;
}

double decimalToDouble(const Decimal& x) {
    auto triple = x.as_uint128_triple();

    switch (triple.tag) {
        case MPD_TRIPLE_INF:
            return triple.sign == 0 ? std::numeric_limits<double>::infinity()
                                    : -std::numeric_limits<double>::infinity();

        case MPD_TRIPLE_QNAN:
        case MPD_TRIPLE_ERROR:
            return std::numeric_limits<double>::quiet_NaN();

        case MPD_TRIPLE_SNAN:
            return std::numeric_limits<double>::signaling_NaN();

        case MPD_TRIPLE_NORMAL: {
            double hi = triple.hi;
            double lo = triple.lo;
            double exp = triple.exp;
            return (triple.sign == 0 ? 1 : -1) * (hi * pow(2, 64) + lo) * pow(10, exp);
            break;
        }
    }

    assert(false);
    return std::numeric_limits<double>::quiet_NaN();
}

}  // namespace util
