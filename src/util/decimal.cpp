#include "util/decimal.h"

using decimal::Decimal;

namespace util {

const decimal::Decimal kDecimalNegativeOne{ -1 };
const decimal::Decimal kDecimalZero{ 0 };
const decimal::Decimal kDecimalOne{ 1 };
const decimal::Decimal kDecimalTwo{ 2 };
const decimal::Decimal kDecimalDoubleMantissaDenominator{ 1ULL << 52 };

size_t getDecimalHash(const Decimal& x) {
    auto triple = x.as_uint128_triple();
    size_t hash = 17;
    hash = 37 * hash + triple.hi;
    hash = 37 * hash + triple.lo;
    hash = 37 * hash + triple.exp;
    hash = 37 * hash + triple.sign;
    hash = 37 * hash + triple.tag;
    return hash;
}

Decimal parseDecimalString(const std::string& str) {
    return Decimal(str);
}

std::string decimalToString(const Decimal& x) {
    if (x.isinfinite()) {
        return x < 0 ? "-Inf" : "Inf";
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
        mpd_uint128_triple_t nanTriple{};
        nanTriple.tag = MPD_TRIPLE_QNAN;
        return Decimal(nanTriple);
    }

    // Decompose double using bit manipulation
    uint64_t bits = 0;
    std::memcpy(&bits, &x, sizeof(double));
    uint32_t negative = (bits >> 63) & 1U;
    uint32_t exponent = (bits >> 52) & 0x7FFU;
    uint64_t mantissa = bits & 0xFFFFFFFFFFFFFU;

    if (std::isinf(x)) {
        mpd_uint128_triple_t infTriple{};
        infTriple.tag = MPD_TRIPLE_INF;
        infTriple.sign = negative;
        return Decimal(infTriple);
    }

    int64_t binaryExponent = exponent;
    binaryExponent -= 0x3ff;  // IEEE 754 double bias

    Decimal fraction = mantissa / kDecimalDoubleMantissaDenominator;

    Decimal magnitude = 2;
    auto isSubnormal = exponent == 0U;
    if (isSubnormal) {
        // Subnormal number
        binaryExponent = -0x3fe;
    }
    magnitude = magnitude.pow(binaryExponent);

    Decimal sign = negative == 0U ? 1 : -1;

    if (exponent == 0U && mantissa == 0U) {
        return kDecimalZero;
    }

    return sign * (isSubnormal ? fraction : (fraction + 1)) * magnitude;
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

decimal::Decimal round(const decimal::Decimal& x) {
    decimal::Decimal xDec{ x };
    auto* xMpd = xDec.get();
    decimal::context.round(MPD_ROUND_HALF_EVEN);
    decimal::Decimal roundedDec{};
    auto* roundedMpd = roundedDec.get();
    mpd_round_to_int(roundedMpd, xMpd, decimal::context.get());
    return roundedDec;
}

}  // namespace util
