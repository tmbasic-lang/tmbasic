#pragma once

#include "../common.h"
#include "Record.h"
#include "String.h"
#include "Value.h"

namespace vm {

const int kMillisecondsPerDay = 24 * 60 * 60 * 1000;
const int kMillisecondsPerHour = 60 * 60 * 1000;
const int kMillisecondsPerMinute = 60 * 1000;
const int kMillisecondsPerSecond = 1000;

class DateTimeParts {
   public:
    uint32_t year;
    uint32_t month;
    uint32_t day;
    uint32_t hour;
    uint32_t minute;
    uint32_t second;
    uint32_t millisecond;

    inline DateTimeParts() = default;
    DateTimeParts(
        uint32_t year,
        uint32_t month,
        uint32_t day,
        uint32_t hour,
        uint32_t minute,
        uint32_t second,
        uint32_t millisecond);
    DateTimeParts(const absl::CivilSecond& civilSecond, uint32_t millisecond);
    explicit DateTimeParts(const Value& value);
    void addYears(int32_t years);
    void addMonths(int32_t months);
    void addDays(int32_t days);
    void addHours(int32_t hours);
    void addMinutes(int32_t minutes);
    void addSeconds(int32_t seconds);
    void addMilliseconds(int32_t milliseconds);
    virtual Value toValue() const;
};

class DateTimeOffsetParts : public DateTimeParts {
   public:
    inline DateTimeOffsetParts() = default;
    DateTimeOffsetParts(
        uint32_t year,
        uint32_t month,
        uint32_t day,
        uint32_t hour,
        uint32_t minute,
        uint32_t second,
        uint32_t millisecond,
        int64_t utcOffsetMilliseconds);
    explicit DateTimeOffsetParts(const Value& value);
    int64_t utcOffsetMilliseconds;
    Value toValue() const override;
};

// Initialization of zoneinfo data at startup.
void initializeTzdb();

// String formatting
boost::local_shared_ptr<String> dateToString(const Value& date);
boost::local_shared_ptr<String> dateTimeToString(const Value& dateTime);
boost::local_shared_ptr<String> dateTimeOffsetToString(const Value& dateTimeOffset);
boost::local_shared_ptr<String> timeSpanToString(const Value& timeSpan);

// Conversion between date types
Value dateTimeOffsetToDateTime(const Value& dateTimeOffset);
Value dateTimeOffsetToDate(const Value& dateTimeOffset);
Value dateTimeToDate(const Value& dateTime);

}  // namespace vm
