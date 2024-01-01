#include "vm/systemCall.h"
#include "vm/castObject.h"
#include "vm/date.h"
#include "vm/Error.h"
#include "vm/List.h"
#include "vm/String.h"
#include "vm/TimeZone.h"

namespace vm {

static void validateYear(int64_t year) {
    if (year < 0) {
        throw Error(ErrorCode::kInvalidDateTime, "The year must be non-negative.");
    }
}

static void validateMonth(int64_t month) {
    if (month < 1 || month > 12) {
        throw Error(ErrorCode::kInvalidDateTime, "The month must be between 1 and 12.");
    }
}

static void validateDay(int64_t day) {
    if (day < 1 || day > 31) {
        throw Error(ErrorCode::kInvalidDateTime, "The day must be between 1 and 31.");
    }
}

static void validateHour(int64_t hour) {
    if (hour < 0 || hour > 23) {
        throw Error(ErrorCode::kInvalidDateTime, "The hour must be between 0 and 23.");
    }
}

static void validateMinute(int64_t minute) {
    if (minute < 0 || minute > 59) {
        throw Error(ErrorCode::kInvalidDateTime, "The minute must be between 0 and 59.");
    }
}

static void validateSecond(int64_t second) {
    if (second < 0 || second > 59) {
        throw Error(ErrorCode::kInvalidDateTime, "The second must be between 0 and 59.");
    }
}

static void validateMillisecond(int64_t millisecond) {
    if (millisecond < 0 || millisecond > 999) {
        throw Error(ErrorCode::kInvalidDateTime, "The millisecond must be between 0 and 999.");
    }
}

void systemCallDateFromParts(const SystemCallInput& input, SystemCallResult* result) {
    auto year = input.getValue(-3).getInt64();
    validateYear(year);

    auto month = input.getValue(-2).getInt64();
    validateMonth(month);

    auto day = input.getValue(-1).getInt64();
    validateDay(day);

    DateTimeParts parts{
        static_cast<uint32_t>(year), static_cast<uint32_t>(month), static_cast<uint32_t>(day), 0, 0, 0, 0
    };
    result->returnedValue = convertDateTimePartsToValue(parts);
}

void systemCallDateTimeFromParts(const SystemCallInput& input, SystemCallResult* result) {
    auto year = input.getValue(-7).getInt64();
    validateYear(year);

    auto month = input.getValue(-6).getInt64();
    validateMonth(month);

    auto day = input.getValue(-5).getInt64();
    validateDay(day);

    auto hour = input.getValue(-4).getInt64();
    validateHour(hour);

    auto minute = input.getValue(-3).getInt64();
    validateMinute(minute);

    auto second = input.getValue(-2).getInt64();
    validateSecond(second);

    auto millisecond = input.getValue(-1).getInt64();
    validateMillisecond(millisecond);

    DateTimeParts parts{ static_cast<uint32_t>(year),       static_cast<uint32_t>(month),
                         static_cast<uint32_t>(day),        static_cast<uint32_t>(hour),
                         static_cast<uint32_t>(minute),     static_cast<uint32_t>(second),
                         static_cast<uint32_t>(millisecond) };

    result->returnedValue = convertDateTimePartsToValue(parts);
}

void systemCallDateTimeOffsetDay(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateValue = input.getValue(-1);
    auto parts = vm::convertValueToDateTimeOffsetParts(dateValue);
    result->returnedValue = Value{ parts.day };
}

void systemCallDateTimeOffsetFromParts(const SystemCallInput& input, SystemCallResult* result) {
    auto year = input.getValue(-8).getInt64();
    validateYear(year);

    auto month = input.getValue(-7).getInt64();
    validateMonth(month);

    auto day = input.getValue(-6).getInt64();
    validateDay(day);

    auto hour = input.getValue(-5).getInt64();
    validateHour(hour);

    auto minute = input.getValue(-4).getInt64();
    validateMinute(minute);

    auto second = input.getValue(-3).getInt64();
    validateSecond(second);

    auto millisecond = input.getValue(-2).getInt64();
    validateMillisecond(millisecond);

    auto offsetMilliseconds = input.getValue(-1).getInt64();

    DateTimeOffsetParts parts{ { static_cast<uint32_t>(year), static_cast<uint32_t>(month), static_cast<uint32_t>(day),
                                 static_cast<uint32_t>(hour), static_cast<uint32_t>(minute),
                                 static_cast<uint32_t>(second), static_cast<uint32_t>(millisecond) },
                               offsetMilliseconds };

    result->returnedValue = convertDateTimeOffsetPartsToValue(parts);
}

void systemCallDateTimeOffsetHour(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateValue = input.getValue(-1);
    auto parts = vm::convertValueToDateTimeOffsetParts(dateValue);
    result->returnedValue = Value{ parts.hour };
}

void systemCallDateTimeOffsetMillisecond(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateValue = input.getValue(-1);
    auto parts = vm::convertValueToDateTimeOffsetParts(dateValue);
    result->returnedValue = Value{ parts.millisecond };
}

void systemCallDateTimeOffsetMinute(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateValue = input.getValue(-1);
    auto parts = vm::convertValueToDateTimeOffsetParts(dateValue);
    result->returnedValue = Value{ parts.minute };
}

void systemCallDateTimeOffsetMonth(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateValue = input.getValue(-1);
    auto parts = vm::convertValueToDateTimeOffsetParts(dateValue);
    result->returnedValue = Value{ parts.month };
}

void systemCallDateTimeOffsetSecond(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateValue = input.getValue(-1);
    auto parts = vm::convertValueToDateTimeOffsetParts(dateValue);
    result->returnedValue = Value{ parts.second };
}

void systemCallDateTimeToDate(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateTime = input.getValue(-1);
    result->returnedValue = dateTimeToDate(dateTime);
}

void systemCallDateToString(const SystemCallInput& input, SystemCallResult* result) {
    const auto& date = input.getValue(-1);
    result->returnedObject = dateToString(date);
}

void systemCallDateTimeToString(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateTime = input.getValue(-1);
    result->returnedObject = dateTimeToString(dateTime);
}

void systemCallDateTimeOffsetToString(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateTimeOffset = input.getValue(-1);
    result->returnedObject = dateTimeOffsetToString(dateTimeOffset);
}

void systemCallDateTimeOffsetYear(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateValue = input.getValue(-1);
    auto parts = vm::convertValueToDateTimeOffsetParts(dateValue);
    result->returnedValue = Value{ parts.year };
}

void systemCallDateTimeOffsetToDateTime(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateTimeOffset = input.getValue(-1);
    result->returnedValue = dateTimeOffsetToDateTime(dateTimeOffset);
}

void systemCallDateTimeOffsetToDate(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateTimeOffset = input.getValue(-1);
    result->returnedValue = dateTimeOffsetToDate(dateTimeOffset);
}

void systemCallDays(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num * MSEC_PER_DAY;
}

void systemCallHours(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num * MSEC_PER_HOUR;
}

void systemCallMilliseconds(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue = input.getValue(-1);  // already in milliseconds!
}

void systemCallMinutes(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num * MSEC_PER_MINUTE;
}

void systemCallSeconds(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num * MSEC_PER_SECOND;
}

void systemCallTimeSpanToString(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedObject = timeSpanToString(input.getValue(-1));
}

void systemCallTimeZoneToString(const SystemCallInput& input, SystemCallResult* result) {
    const auto& timeZone = *castTimeZone(input.getObject(-1));
    result->returnedObject = boost::make_local_shared<String>(timeZone.zone->name());
}

void systemCallTimeZoneFromName(const SystemCallInput& input, SystemCallResult* result) {
    const auto& name = *castString(input.getObject(-1));

    auto time_zone_ptr = std::make_unique<absl::TimeZone>();
    auto success = absl::LoadTimeZone(name.value, time_zone_ptr.get());
    if (!success) {
        throw Error(ErrorCode::kInvalidTimeZone, "The specified time zone was not found.");
    }

    result->returnedObject = boost::make_local_shared<TimeZone>(std::move(time_zone_ptr));
}

void systemCallTotalDays(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num / MSEC_PER_DAY;
}

void systemCallTotalHours(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num / MSEC_PER_HOUR;
}

void systemCallTotalMilliseconds(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue = input.getValue(-1);  // already in milliseconds!
}

void systemCallTotalMinutes(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num / MSEC_PER_MINUTE;
}

void systemCallTotalSeconds(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num / MSEC_PER_SECOND;
}

void systemCallUtcOffsets(const SystemCallInput& input, SystemCallResult* result) {
    const auto& timeZone = *castTimeZone(input.getObject(-1));
    const auto& dateTimeValue = input.getValue(-1);
    auto dateTimeParts = convertValueToDateTimeParts(dateTimeValue);
    auto offsets = timeZone.getUtcOffsets(dateTimeParts);
    ValueListBuilder builder{};
    for (const auto& offset : offsets) {
        builder.items.push_back(Value{ offset });
    }
    result->returnedObject = boost::make_local_shared<ValueList>(&builder);
}

}  // namespace vm
