#include "../common.h"
#include "gtest/gtest.h"
#include "vm/date.h"

TEST(DateTest, DateTimePartsRoundTrip) {
    vm::DateTimeParts parts{ 2019, 12, 31, 23, 55, 59, 999 };
    auto value = parts.toValue();
    vm::DateTimeParts roundTripParts{ value };
    ASSERT_EQ(parts.year, roundTripParts.year);
    ASSERT_EQ(parts.month, roundTripParts.month);
    ASSERT_EQ(parts.day, roundTripParts.day);
    ASSERT_EQ(parts.hour, roundTripParts.hour);
    ASSERT_EQ(parts.minute, roundTripParts.minute);
    ASSERT_EQ(parts.second, roundTripParts.second);
    ASSERT_EQ(parts.millisecond, roundTripParts.millisecond);
}

TEST(DateTest, DateTimeOffsetPartsRoundTrip) {
    vm::DateTimeOffsetParts parts{ 2019, 12, 31, 23, 55, 59, 999, -6 * vm::kMillisecondsPerHour };
    auto value = parts.toValue();
    vm::DateTimeOffsetParts roundTripParts{ value };
    ASSERT_EQ(parts.year, roundTripParts.year);
    ASSERT_EQ(parts.month, roundTripParts.month);
    ASSERT_EQ(parts.day, roundTripParts.day);
    ASSERT_EQ(parts.hour, roundTripParts.hour);
    ASSERT_EQ(parts.minute, roundTripParts.minute);
    ASSERT_EQ(parts.second, roundTripParts.second);
    ASSERT_EQ(parts.millisecond, roundTripParts.millisecond);
    ASSERT_EQ(parts.utcOffsetMilliseconds, roundTripParts.utcOffsetMilliseconds);
}
