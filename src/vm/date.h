#pragma once

#include "../common.h"
#include "Record.h"
#include "String.h"
#include "Value.h"

namespace vm {

const int MSEC_PER_DAY = 24 * 60 * 60 * 1000;
const int MSEC_PER_HOUR = 60 * 60 * 1000;
const int MSEC_PER_MINUTE = 60 * 1000;
const int MSEC_PER_SECOND = 1000;

class DateTimeParts {
   public:
    uint32_t year;
    uint32_t month;
    uint32_t day;
    uint32_t hour;
    uint32_t minute;
    uint32_t second;
    uint32_t millisecond;
};

class DateTimeOffsetParts : public DateTimeParts {
   public:
    int64_t utcOffsetMilliseconds;
};

// Low-level packing and unpacking.
Value convertDateTimePartsToValue(DateTimeParts parts);
DateTimeParts convertValueToDateTimeParts(Value value);
Value convertDateTimeOffsetPartsToValue(DateTimeOffsetParts parts);
DateTimeOffsetParts convertValueToDateTimeOffsetParts(Value value);

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
