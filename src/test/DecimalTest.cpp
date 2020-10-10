#include "common.h"
#include "gtest/gtest.h"
#include "core/util/decimal.h"

using namespace util;

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
    ASSERT_EQ("12345678901234570", roundtrip("12345678901234567"));
}

TEST(DecimalTest, FractionWith16FiguresAndIntegerZero) {
    ASSERT_EQ("0.1234567890123456", roundtrip("0.1234567890123456"));
}

TEST(DecimalTest, FractionWith16FiguresWithoutIntegerZero) {
    ASSERT_EQ("0.1234567890123456", roundtrip(".1234567890123456"));
}

TEST(DecimalTest, FractionWith17Figures) {
    ASSERT_EQ("0.1234567890123457", roundtrip("0.12345678901234567"));
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
    ASSERT_EQ("-12345678901234570", roundtrip("-12345678901234567"));
}

TEST(DecimalTest, NegativeFractionWith16FiguresAndIntegerZero) {
    ASSERT_EQ("-0.1234567890123456", roundtrip("-0.1234567890123456"));
}

TEST(DecimalTest, NegativeFractionWith16FiguresWithoutIntegerZero) {
    ASSERT_EQ("-0.1234567890123456", roundtrip("-.1234567890123456"));
}

TEST(DecimalTest, NegativeFractionWith17Figures) {
    ASSERT_EQ("-0.1234567890123457", roundtrip("-0.12345678901234567"));
}

TEST(DecimalTest, NegativeZero) {
    ASSERT_EQ("0", roundtrip("-0"));
}

TEST(DecimalTest, NegativeManyZeroes) {
    ASSERT_EQ("0", roundtrip("-00000"));
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
