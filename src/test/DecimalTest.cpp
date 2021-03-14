#include "../common.h"
#include "gtest/gtest.h"
#include "util/decimal.h"

using decimal::Decimal;
using util::decimalToDouble;
using util::decimalToString;
using util::doubleToDecimal;
using util::parseDecimalString;

static std::string roundtrip(std::string str) {
    return decimalToString(parseDecimalString(str));
}

TEST(DecimalTest, SmallInteger) {
    ASSERT_EQ("1234567890", roundtrip("1234567890"));
}

TEST(DecimalTest, SmallIntegerWithLeadingZero) {
    ASSERT_EQ("1234567890", roundtrip("0001234567890"));
}

TEST(DecimalTest, SmallDecimalWithTrailingZero) {
    ASSERT_EQ("123456.789", roundtrip("123456.7890"));
}

TEST(DecimalTest, SmallDecimalWithLeadingZeroAndTrailingZero) {
    ASSERT_EQ("123456.789", roundtrip("0123456.7890"));
}

TEST(DecimalTest, IntegerWith16Figures) {
    ASSERT_EQ("1234567890123456", roundtrip("1234567890123456"));
}

TEST(DecimalTest, IntegerWith17Figures) {
    ASSERT_EQ("12345678901234567", roundtrip("12345678901234567"));
}

TEST(DecimalTest, FractionWith16FiguresAndIntegerZero) {
    ASSERT_EQ("0.1234567890123456", roundtrip("0.1234567890123456"));
}

TEST(DecimalTest, FractionWith16FiguresWithoutIntegerZero) {
    ASSERT_EQ("0.1234567890123456", roundtrip(".1234567890123456"));
}

TEST(DecimalTest, FractionWith17Figures) {
    ASSERT_EQ("0.12345678901234567", roundtrip("0.12345678901234567"));
}

TEST(DecimalTest, Zero) {
    ASSERT_EQ("0", roundtrip("0"));
}

TEST(DecimalTest, ManyZeroes) {
    ASSERT_EQ("0", roundtrip("00000"));
}

TEST(DecimalTest, NegativeSmallInteger) {
    ASSERT_EQ("-1234567890", roundtrip("-1234567890"));
}

TEST(DecimalTest, NegativeSmallIntegerWithLeadingZero) {
    ASSERT_EQ("-1234567890", roundtrip("-0001234567890"));
}

TEST(DecimalTest, NegativeSmallDecimalWithTrailingZero) {
    ASSERT_EQ("-123456.789", roundtrip("-123456.7890"));
}

TEST(DecimalTest, NegativeSmallDecimalWithLeadingZeroAndTrailingZero) {
    ASSERT_EQ("-123456.789", roundtrip("-0123456.7890"));
}

TEST(DecimalTest, NegativeIntegerWith16Figures) {
    ASSERT_EQ("-1234567890123456", roundtrip("-1234567890123456"));
}

TEST(DecimalTest, NegativeIntegerWith17Figures) {
    ASSERT_EQ("-12345678901234567", roundtrip("-12345678901234567"));
}

TEST(DecimalTest, NegativeFractionWith16FiguresAndIntegerZero) {
    ASSERT_EQ("-0.1234567890123456", roundtrip("-0.1234567890123456"));
}

TEST(DecimalTest, NegativeFractionWith16FiguresWithoutIntegerZero) {
    ASSERT_EQ("-0.1234567890123456", roundtrip("-.1234567890123456"));
}

TEST(DecimalTest, NegativeFractionWith17Figures) {
    ASSERT_EQ("-0.12345678901234567", roundtrip("-0.12345678901234567"));
}

TEST(DecimalTest, NegativeZero) {
    ASSERT_EQ("-0", roundtrip("-0"));
}

TEST(DecimalTest, NegativeManyZeroes) {
    ASSERT_EQ("-0", roundtrip("-00000"));
}

TEST(DecimalTest, PositiveInfinity) {
    ASSERT_EQ("Inf", roundtrip("inf"));
}

TEST(DecimalTest, NegativeInfinity) {
    ASSERT_EQ("-Inf", roundtrip("-inf"));
}

TEST(DecimalTest, NotANumber) {
    ASSERT_EQ("NaN", roundtrip("nan"));
}

TEST(DecimalTest, DoubleToDecimal1) {
    auto actual = doubleToDecimal(1);
    Decimal expected = 1;
    ASSERT_EQ(expected, actual);
}

TEST(DecimalTest, DoubleToDecimal1_5) {
    auto actual = doubleToDecimal(1.5);
    Decimal expected = 3;
    expected /= 2;
    ASSERT_EQ(expected, actual);
}

TEST(DecimalTest, DoubleToDecimal0) {
    auto actual = doubleToDecimal(0);
    Decimal expected = 0;
    ASSERT_EQ(expected, actual);
}

TEST(DecimalTest, DoubleToDecimalNeg0) {
    auto actual = doubleToDecimal(-0);
    Decimal expected = -0;
    ASSERT_EQ(expected, actual);
}

TEST(DecimalTest, DoubleToDecimalNaN) {
    auto actual = doubleToDecimal(std::nan(""));
    ASSERT_TRUE(actual.isnan());
}

TEST(DecimalTest, DoubleToDecimalInf) {
    auto actual = doubleToDecimal(std::numeric_limits<double>::infinity());
    ASSERT_TRUE(actual.isinfinite());
    ASSERT_EQ(1, actual.sign());
}

TEST(DecimalTest, DoubleToDecimalNegInf) {
    auto actual = doubleToDecimal(-std::numeric_limits<double>::infinity());
    ASSERT_TRUE(actual.isinfinite());
    ASSERT_EQ(-1, actual.sign());
}

TEST(DecimalTest, DoubleToDecimal1234567_875) {
    auto actual = doubleToDecimal(1234567.875);
    auto expected = Decimal(1234567) + Decimal(875) / Decimal(1000);
    ASSERT_EQ(expected, actual);
}

TEST(DecimalTest, DoubleToDecimalNeg1234567_875) {
    auto actual = doubleToDecimal(-1234567.875);
    auto expected = Decimal(-1) * (Decimal(1234567) + Decimal(875) / Decimal(1000));
    ASSERT_EQ(expected, actual);
}

TEST(DecimalTest, DecimalToDouble1) {
    auto actual = decimalToDouble(Decimal("1"));
    double expected = 1;
    ASSERT_EQ(expected, actual);
}

TEST(DecimalTest, DecimalToDouble1_5) {
    auto actual = decimalToDouble(Decimal("1.5"));
    double expected = 1.5;
    ASSERT_EQ(expected, actual);
}

TEST(DecimalTest, DecimalToDouble0) {
    auto actual = decimalToDouble(Decimal("0"));
    double expected = 0;
    ASSERT_EQ(expected, actual);
}

TEST(DecimalTest, DecimalToDoubleNeg0) {
    auto actual = decimalToDouble(Decimal("-0"));
    double expected = -0;
    ASSERT_EQ(expected, actual);
}

TEST(DecimalTest, DecimalToDoubleNaN) {
    mpd_uint128_triple_t nanTriple;
    memset(&nanTriple, 0, sizeof(nanTriple));
    nanTriple.tag = MPD_TRIPLE_QNAN;
    auto dec = Decimal(nanTriple);
    auto actual = decimalToDouble(dec);
    ASSERT_TRUE(std::isnan(actual));
}

TEST(DecimalTest, DecimalToDoubleInf) {
    mpd_uint128_triple_t infTriple;
    memset(&infTriple, 0, sizeof(infTriple));
    infTriple.tag = MPD_TRIPLE_INF;
    infTriple.sign = 0;
    auto dec = Decimal(infTriple);
    auto actual = decimalToDouble(dec);
    ASSERT_TRUE(std::isinf(actual));
    ASSERT_TRUE(actual > 0);
}

TEST(DecimalTest, DecimalToDoubleNegInf) {
    mpd_uint128_triple_t infTriple;
    memset(&infTriple, 0, sizeof(infTriple));
    infTriple.tag = MPD_TRIPLE_INF;
    infTriple.sign = 1;
    auto dec = Decimal(infTriple);
    auto actual = decimalToDouble(dec);
    ASSERT_TRUE(std::isinf(actual));
    ASSERT_TRUE(actual < 0);
}

TEST(DecimalTest, DecimalToDouble1234567_875) {
    auto actual = decimalToDouble(Decimal("1234567.875"));
    ASSERT_EQ(1234567.875, actual);
}

TEST(DecimalTest, DecimalToDoubleNeg1234567_875) {
    auto actual = decimalToDouble(Decimal("-1234567.875"));
    ASSERT_EQ(-1234567.875, actual);
}
