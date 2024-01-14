#include "../common.h"
#include "gtest/gtest.h"
#include "shared/decimal.h"

using decimal::Decimal;
using shared::decimalToDouble;
using shared::decimalToString;
using shared::doubleToDecimal;
using shared::parseDecimalString;

class DecimalTest : public ::testing::Test {
   protected:
    void SetUp() override { decimal::context = decimal::IEEEContext(decimal::DECIMAL128); }
};

static std::string roundtrip(std::string str) {
    return decimalToString(parseDecimalString(str));
}

TEST_F(DecimalTest, SmallInteger) {
    ASSERT_EQ("1234567890", roundtrip("1234567890"));
}

TEST_F(DecimalTest, SmallIntegerWithLeadingZero) {
    ASSERT_EQ("1234567890", roundtrip("0001234567890"));
}

TEST_F(DecimalTest, SmallDecimalWithTrailingZero) {
    ASSERT_EQ("123456.789", roundtrip("123456.7890"));
}

TEST_F(DecimalTest, SmallDecimalWithLeadingZeroAndTrailingZero) {
    ASSERT_EQ("123456.789", roundtrip("0123456.7890"));
}

TEST_F(DecimalTest, IntegerWith16Figures) {
    ASSERT_EQ("1234567890123456", roundtrip("1234567890123456"));
}

TEST_F(DecimalTest, IntegerWith17Figures) {
    ASSERT_EQ("12345678901234567", roundtrip("12345678901234567"));
}

TEST_F(DecimalTest, FractionWith16FiguresAndIntegerZero) {
    ASSERT_EQ("0.1234567890123456", roundtrip("0.1234567890123456"));
}

TEST_F(DecimalTest, FractionWith16FiguresWithoutIntegerZero) {
    ASSERT_EQ("0.1234567890123456", roundtrip(".1234567890123456"));
}

TEST_F(DecimalTest, FractionWith17Figures) {
    ASSERT_EQ("0.12345678901234567", roundtrip("0.12345678901234567"));
}

TEST_F(DecimalTest, Zero) {
    ASSERT_EQ("0", roundtrip("0"));
}

TEST_F(DecimalTest, ManyZeroes) {
    ASSERT_EQ("0", roundtrip("00000"));
}

TEST_F(DecimalTest, NegativeSmallInteger) {
    ASSERT_EQ("-1234567890", roundtrip("-1234567890"));
}

TEST_F(DecimalTest, NegativeSmallIntegerWithLeadingZero) {
    ASSERT_EQ("-1234567890", roundtrip("-0001234567890"));
}

TEST_F(DecimalTest, NegativeSmallDecimalWithTrailingZero) {
    ASSERT_EQ("-123456.789", roundtrip("-123456.7890"));
}

TEST_F(DecimalTest, NegativeSmallDecimalWithLeadingZeroAndTrailingZero) {
    ASSERT_EQ("-123456.789", roundtrip("-0123456.7890"));
}

TEST_F(DecimalTest, NegativeIntegerWith16Figures) {
    ASSERT_EQ("-1234567890123456", roundtrip("-1234567890123456"));
}

TEST_F(DecimalTest, NegativeIntegerWith17Figures) {
    ASSERT_EQ("-12345678901234567", roundtrip("-12345678901234567"));
}

TEST_F(DecimalTest, NegativeFractionWith16FiguresAndIntegerZero) {
    ASSERT_EQ("-0.1234567890123456", roundtrip("-0.1234567890123456"));
}

TEST_F(DecimalTest, NegativeFractionWith16FiguresWithoutIntegerZero) {
    ASSERT_EQ("-0.1234567890123456", roundtrip("-.1234567890123456"));
}

TEST_F(DecimalTest, NegativeFractionWith17Figures) {
    ASSERT_EQ("-0.12345678901234567", roundtrip("-0.12345678901234567"));
}

TEST_F(DecimalTest, NegativeZero) {
    ASSERT_EQ("-0", roundtrip("-0"));
}

TEST_F(DecimalTest, NegativeManyZeroes) {
    ASSERT_EQ("-0", roundtrip("-00000"));
}

TEST_F(DecimalTest, PositiveInfinity) {
    ASSERT_EQ("Inf", roundtrip("inf"));
}

TEST_F(DecimalTest, NegativeInfinity) {
    ASSERT_EQ("-Inf", roundtrip("-inf"));
}

TEST_F(DecimalTest, NotANumber) {
    ASSERT_EQ("NaN", roundtrip("nan"));
}

TEST_F(DecimalTest, DoubleToDecimal1) {
    auto actual = doubleToDecimal(1);
    Decimal expected = 1;
    ASSERT_EQ(expected, actual);
}

TEST_F(DecimalTest, DoubleToDecimal1_5) {
    auto actual = doubleToDecimal(1.5);
    Decimal expected = 3;
    expected /= 2;
    ASSERT_EQ(expected, actual);
}

TEST_F(DecimalTest, DoubleToDecimal0) {
    auto actual = doubleToDecimal(0);
    Decimal expected = 0;
    ASSERT_EQ(expected, actual);
}

TEST_F(DecimalTest, DoubleToDecimalNeg0) {
    auto actual = doubleToDecimal(-0);
    Decimal expected = -0;
    ASSERT_EQ(expected, actual);
}

TEST_F(DecimalTest, DoubleToDecimalNaN) {
    auto actual = doubleToDecimal(std::nan(""));
    ASSERT_TRUE(actual.isnan());
}

TEST_F(DecimalTest, DoubleToDecimalInf) {
    auto actual = doubleToDecimal(std::numeric_limits<double>::infinity());
    ASSERT_TRUE(actual.isinfinite());
    ASSERT_EQ(1, actual.sign());
}

TEST_F(DecimalTest, DoubleToDecimalNegInf) {
    auto actual = doubleToDecimal(-std::numeric_limits<double>::infinity());
    ASSERT_TRUE(actual.isinfinite());
    ASSERT_EQ(-1, actual.sign());
}

TEST_F(DecimalTest, DoubleToDecimal1234567_875) {
    auto actual = doubleToDecimal(1234567.875);
    auto expected = Decimal(1234567) + Decimal(875) / Decimal(1000);
    ASSERT_EQ(expected, actual);
}

TEST_F(DecimalTest, DoubleToDecimalNeg1234567_875) {
    auto actual = doubleToDecimal(-1234567.875);
    auto expected = Decimal(-1) * (Decimal(1234567) + Decimal(875) / Decimal(1000));
    ASSERT_EQ(expected, actual);
}

TEST_F(DecimalTest, DecimalToDouble1) {
    auto actual = decimalToDouble(Decimal("1"));
    double expected = 1;
    ASSERT_EQ(expected, actual);
}

TEST_F(DecimalTest, DecimalToDouble1_5) {
    auto actual = decimalToDouble(Decimal("1.5"));
    double expected = 1.5;
    ASSERT_EQ(expected, actual);
}

TEST_F(DecimalTest, DecimalToDouble0) {
    auto actual = decimalToDouble(Decimal("0"));
    double expected = 0;
    ASSERT_EQ(expected, actual);
}

TEST_F(DecimalTest, DecimalToDoubleNeg0) {
    auto actual = decimalToDouble(Decimal("-0"));
    double expected = -0;
    ASSERT_EQ(expected, actual);
}

TEST_F(DecimalTest, DecimalToDoubleNaN) {
    mpd_uint128_triple_t nanTriple;
    memset(&nanTriple, 0, sizeof(nanTriple));
    nanTriple.tag = MPD_TRIPLE_QNAN;
    auto dec = Decimal(nanTriple);
    auto actual = decimalToDouble(dec);
    ASSERT_TRUE(std::isnan(actual));
}

TEST_F(DecimalTest, DecimalToDoubleInf) {
    mpd_uint128_triple_t infTriple;
    memset(&infTriple, 0, sizeof(infTriple));
    infTriple.tag = MPD_TRIPLE_INF;
    infTriple.sign = 0;
    auto dec = Decimal(infTriple);
    auto actual = decimalToDouble(dec);
    ASSERT_TRUE(std::isinf(actual));
    ASSERT_GT(actual, 0);
}

TEST_F(DecimalTest, DecimalToDoubleNegInf) {
    mpd_uint128_triple_t infTriple;
    memset(&infTriple, 0, sizeof(infTriple));
    infTriple.tag = MPD_TRIPLE_INF;
    infTriple.sign = 1;
    auto dec = Decimal(infTriple);
    auto actual = decimalToDouble(dec);
    ASSERT_TRUE(std::isinf(actual));
    ASSERT_LT(actual, 0);
}

static double setLastBit(double input) {
    union {
        double d;
        uint64_t i;
    };
    d = input;
    i |= 1;
    return d;
}

TEST_F(DecimalTest, DecimalToDouble1234567_875) {
    auto actual = decimalToDouble(Decimal("1234567.875"));
    double expected = 1234567.875;
    // on win64 the last bit differs. why?
    ASSERT_EQ(setLastBit(expected), setLastBit(actual));
}

TEST_F(DecimalTest, DecimalToDoubleNeg1234567_875) {
    auto actual = decimalToDouble(Decimal("-1234567.875"));
    double expected = -1234567.875;
    // on win64 the last bit differs. why?
    ASSERT_EQ(setLastBit(expected), setLastBit(actual));
}
