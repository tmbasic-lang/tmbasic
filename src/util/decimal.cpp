#include "util/decimal.h"

namespace util {

size_t getDecimalHash(const decimal::Decimal& x) {
    return std::hash<int64_t>{}(x.floor().i64());
}

decimal::Decimal parseDecimalString(const std::string& str) {
    return decimal::Decimal(str);
}

std::string decimalToString(const decimal::Decimal& x) {
    if (x.isinfinite()) {
        return x < decimal::Decimal(0) ? "-Inf" : "Inf";
    }
    if (x.isnan()) {
        return "NaN";
    }

    return x.format("f");
}

decimal::Decimal doubleToDecimal(double x) {
    if (std::isnan(x)) {
        mpd_uint128_triple_t nanTriple;
        memset(&nanTriple, 0, sizeof(nanTriple));
        nanTriple.tag = MPD_TRIPLE_QNAN;
        return decimal::Decimal(nanTriple);
    }

    ieee754_double decomposed {};
    decomposed.d = x;

    if (std::isinf(x)) {
        mpd_uint128_triple_t infTriple;
        memset(&infTriple, 0, sizeof(infTriple));
        infTriple.tag = MPD_TRIPLE_INF;
        infTriple.sign = decomposed.ieee.negative;
        return decimal::Decimal(infTriple);
    }

    int64_t binaryExponent = decomposed.ieee.exponent;
    binaryExponent -= IEEE754_DOUBLE_BIAS;

    uint64_t mantissa = decomposed.ieee.mantissa0;
    mantissa <<= 32;
    mantissa |= decomposed.ieee.mantissa1;

    decimal::Decimal fractionNumerator = mantissa;

    decimal::Decimal fractionDenominator = 2;
    fractionDenominator = fractionDenominator.pow(52);

    decimal::Decimal fraction = fractionNumerator / fractionDenominator;

    decimal::Decimal magnitude = 2;
    magnitude = magnitude.pow(binaryExponent);

    decimal::Decimal sign = decomposed.ieee.negative == 0U ? 1 : -1;

    if (mantissa == 0 && decomposed.ieee.exponent == 0U) {
        return sign * fraction * magnitude;
    }
    return sign * (fraction + 1) * magnitude;
}

}  // namespace util
