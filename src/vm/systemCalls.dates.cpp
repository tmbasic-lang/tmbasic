#include "vm/systemCall.h"
#include "shared/Error.h"
#include "vm/castObject.h"
#include "vm/date.h"
#include "vm/List.h"
#include "vm/String.h"
#include "vm/TimeZone.h"

using shared::Error;
using shared::ErrorCode;

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

// (year as Number, month as Number, day as Number) as Date
void systemCallDateFromParts(const SystemCallInput& input, SystemCallResult* result) {
    auto year = input.getValue(-3).getInt64();
    validateYear(year);

    auto month = input.getValue(-2).getInt64();
    validateMonth(month);

    auto day = input.getValue(-1).getInt64();
    validateDay(day);

    DateTimeParts const parts{
        static_cast<uint32_t>(year), static_cast<uint32_t>(month), static_cast<uint32_t>(day), 0, 0, 0, 0
    };
    result->returnedValue = parts.toValue();
}

// (year as Number, ...) as DateTime
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

    DateTimeParts const parts{ static_cast<uint32_t>(year),       static_cast<uint32_t>(month),
                               static_cast<uint32_t>(day),        static_cast<uint32_t>(hour),
                               static_cast<uint32_t>(minute),     static_cast<uint32_t>(second),
                               static_cast<uint32_t>(millisecond) };

    result->returnedValue = parts.toValue();
}

// (dateTimeOffset as DateTimeOffset) as Number
void systemCallDateTimeOffsetDay(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateValue = input.getValue(-1);
    DateTimeOffsetParts const parts{ dateValue };
    result->returnedValue = Value{ parts.day };
}

// (year as N ... utcOffset as TimeSpan) as DateTimeOffset
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

    DateTimeOffsetParts const parts{ static_cast<uint32_t>(year),        static_cast<uint32_t>(month),
                                     static_cast<uint32_t>(day),         static_cast<uint32_t>(hour),
                                     static_cast<uint32_t>(minute),      static_cast<uint32_t>(second),
                                     static_cast<uint32_t>(millisecond), offsetMilliseconds };

    result->returnedValue = parts.toValue();
}

// (dateTimeOffset as DateTimeOffset) as Number
void systemCallDateTimeOffsetHour(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateValue = input.getValue(-1);
    DateTimeOffsetParts const parts{ dateValue };
    result->returnedValue = Value{ parts.hour };
}

// (dateTimeOffset as DateTimeOffset) as Number
void systemCallDateTimeOffsetMillisecond(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateValue = input.getValue(-1);
    DateTimeOffsetParts const parts{ dateValue };
    result->returnedValue = Value{ parts.millisecond };
}

// (dateTimeOffset as DateTimeOffset) as Number
void systemCallDateTimeOffsetMinute(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateValue = input.getValue(-1);
    DateTimeOffsetParts const parts{ dateValue };
    result->returnedValue = Value{ parts.minute };
}

// (dateTimeOffset as DateTimeOffset) as Number
void systemCallDateTimeOffsetMonth(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateValue = input.getValue(-1);
    DateTimeOffsetParts const parts{ dateValue };
    result->returnedValue = Value{ parts.month };
}

// (dateTimeOffset as DateTimeOffset) as Number
void systemCallDateTimeOffsetSecond(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateValue = input.getValue(-1);
    DateTimeOffsetParts const parts{ dateValue };
    result->returnedValue = Value{ parts.second };
}

// (dateTime as DateTime) as Date
void systemCallDateTimeToDate(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateTime = input.getValue(-1);
    result->returnedValue = dateTimeToDate(dateTime);
}

// (date as Date) as String
void systemCallDateToString(const SystemCallInput& input, SystemCallResult* result) {
    const auto& date = input.getValue(-1);
    result->returnedObject = dateToString(date);
}

// (dateTime as DateTime) as String
void systemCallDateTimeToString(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateTime = input.getValue(-1);
    result->returnedObject = dateTimeToString(dateTime);
}

// (dateTimeOffset as DateTimeOffset) as String
void systemCallDateTimeOffsetToString(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateTimeOffset = input.getValue(-1);
    result->returnedObject = dateTimeOffsetToString(dateTimeOffset);
}

// (dateTimeOffset as DateTimeOffset) as Number
void systemCallDateTimeOffsetYear(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateValue = input.getValue(-1);
    DateTimeOffsetParts const parts{ dateValue };
    result->returnedValue = Value{ parts.year };
}

// (dateTimeOffset as DateTimeOffset) as DateTime
void systemCallDateTimeOffsetToDateTime(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateTimeOffset = input.getValue(-1);
    result->returnedValue = dateTimeOffsetToDateTime(dateTimeOffset);
}

// (dateTimeOffset as DateTimeOffset) as Date
void systemCallDateTimeOffsetToDate(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateTimeOffset = input.getValue(-1);
    result->returnedValue = dateTimeOffsetToDate(dateTimeOffset);
}

// (count as Number) as TimeSpan
void systemCallDays(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num * kMillisecondsPerDay;
}

// (count as Number) as TimeSpan
void systemCallHours(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num * kMillisecondsPerHour;
}

// (count as Number) as TimeSpan
void systemCallMilliseconds(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue = input.getValue(-1);  // already in milliseconds!
}

// (count as Number) as TimeSpan
void systemCallMinutes(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num * kMillisecondsPerMinute;
}

// (count as Number) as TimeSpan
void systemCallSeconds(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num * kMillisecondsPerSecond;
}

// () as DateTimeOffset
void systemCallNow(const SystemCallInput& /*input*/, SystemCallResult* result) {
    absl::TimeZone tz;
#ifdef _WIN32
    // Ask Windows for the local time zone because Abseil doesn't know how to do it.
    DYNAMIC_TIME_ZONE_INFORMATION dynamicTimeZoneInfo{};
    switch (GetDynamicTimeZoneInformation(&dynamicTimeZoneInfo)) {
        case TIME_ZONE_ID_DAYLIGHT:
            tz = absl::FixedTimeZone(
                static_cast<int>(-dynamicTimeZoneInfo.Bias - dynamicTimeZoneInfo.DaylightBias) * 60);
            break;
        default:
            tz = absl::FixedTimeZone(static_cast<int>(-dynamicTimeZoneInfo.Bias) * 60);
            break;
    }
#else
    // Trust Abseil on other platforms.
    tz = absl::LocalTimeZone();
#endif

    // Get the UTC time.
    auto time = absl::Now();

    // Convert to CivilSecond in the local time zone.
    auto civilTime = absl::ToCivilSecond(time, tz);

    // Extract year, month, day, hour, minute, and second as integers.
    auto year = civilTime.year();
    auto month = civilTime.month();
    auto day = civilTime.day();
    auto hour = civilTime.hour();
    auto minute = civilTime.minute();
    auto second = civilTime.second();

    // Calculate milliseconds.
    auto sinceCivil = time - absl::FromCivil(civilTime, tz);
    auto milliseconds = absl::ToInt64Milliseconds(sinceCivil) % 1000;

    // Get the UTC offset in milliseconds
    auto civilInfo = tz.At(time);
    auto utcOffsetMilliseconds = static_cast<int64_t>(civilInfo.offset) * 1000;

    // Construct DateTimeOffsetParts.
    DateTimeOffsetParts const parts{ static_cast<uint32_t>(year),         static_cast<uint32_t>(month),
                                     static_cast<uint32_t>(day),          static_cast<uint32_t>(hour),
                                     static_cast<uint32_t>(minute),       static_cast<uint32_t>(second),
                                     static_cast<uint32_t>(milliseconds), utcOffsetMilliseconds };

    // Return it in Value format.
    result->returnedValue = parts.toValue();
}

// (timeSpan as TimeSpan) as String
void systemCallTimeSpanToString(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedObject = timeSpanToString(input.getValue(-1));
}

// (timeZone as TimeZone) as String
void systemCallTimeZoneToString(const SystemCallInput& input, SystemCallResult* result) {
    const auto& timeZone = *castTimeZone(input.getObject(-1));
    result->returnedObject = boost::make_intrusive_ptr<String>(timeZone.zone->name());
}

// (name as String) as TimeZone
void systemCallTimeZoneFromName(const SystemCallInput& input, SystemCallResult* result) {
    const auto& name = *castString(input.getObject(-1));

    auto timeZonePtr = std::make_unique<absl::TimeZone>();
    auto success = absl::LoadTimeZone(name.value, timeZonePtr.get());
    if (!success) {
        throw Error(ErrorCode::kInvalidTimeZone, "The specified time zone was not found.");
    }

    result->returnedObject = boost::make_intrusive_ptr<TimeZone>(std::move(timeZonePtr));
}

// (timeSpan as TimeSpan) as Number
void systemCallTotalDays(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num / kMillisecondsPerDay;
}

// (timeSpan as TimeSpan) as Number
void systemCallTotalHours(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num / kMillisecondsPerHour;
}

// (timeSpan as TimeSpan) as Number
void systemCallTotalMilliseconds(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue = input.getValue(-1);  // already in milliseconds!
}

// (timeSpan as TimeSpan) as Number
void systemCallTotalMinutes(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num / kMillisecondsPerMinute;
}

// (timeSpan as TimeSpan) as Number
void systemCallTotalSeconds(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num / kMillisecondsPerSecond;
}

// (timeZone as TimeZone, dateTime as DateTime) as List of TimeSpan
void systemCallUtcOffsets(const SystemCallInput& input, SystemCallResult* result) {
    const auto& timeZone = *castTimeZone(input.getObject(-1));
    const auto& dateTimeValue = input.getValue(-1);
    DateTimeParts const dateTimeParts{ dateTimeValue };
    auto offsets = timeZone.getUtcOffsets(dateTimeParts);
    ValueListBuilder builder{};
    for (const auto& offset : offsets) {
        builder.items.push_back(Value{ offset });
    }
    result->returnedObject = boost::make_intrusive_ptr<ValueList>(&builder);
}

// AddYears, AddMonths, AddDays, etc. are all the same except for the operation performed.
template <typename Parts, typename Operation>
void dateTimeAddCore(Operation operation, const SystemCallInput& input, SystemCallResult* result) {
    const auto& value = input.getValue(-2);
    auto count = input.getValue(-1).getInt64();
    Parts parts{ value };
    operation(&parts, count);
    result->returnedValue = parts.toValue();
}

// (dto as DateTimeOffset, count as Number) as DateTimeOffset
void systemCallDateTimeOffsetAddYears(const SystemCallInput& input, SystemCallResult* result) {
    dateTimeAddCore<DateTimeOffsetParts>([](auto* parts, auto count) { parts->addYears(count); }, input, result);
}

// (dto as DateTimeOffset, count as Number) as DateTimeOffset
void systemCallDateTimeOffsetAddMonths(const SystemCallInput& input, SystemCallResult* result) {
    dateTimeAddCore<DateTimeOffsetParts>([](auto* parts, auto count) { parts->addMonths(count); }, input, result);
}

// (dto as DateTimeOffset, count as Number) as DateTimeOffset
void systemCallDateTimeOffsetAddDays(const SystemCallInput& input, SystemCallResult* result) {
    dateTimeAddCore<DateTimeOffsetParts>([](auto* parts, auto count) { parts->addDays(count); }, input, result);
}

// (dto as DateTimeOffset, count as Number) as DateTimeOffset
void systemCallDateTimeOffsetAddHours(const SystemCallInput& input, SystemCallResult* result) {
    dateTimeAddCore<DateTimeOffsetParts>([](auto* parts, auto count) { parts->addHours(count); }, input, result);
}

// (dto as DateTimeOffset, count as Number) as DateTimeOffset
void systemCallDateTimeOffsetAddMinutes(const SystemCallInput& input, SystemCallResult* result) {
    dateTimeAddCore<DateTimeOffsetParts>([](auto* parts, auto count) { parts->addMinutes(count); }, input, result);
}

// (dto as DateTimeOffset, count as Number) as DateTimeOffset
void systemCallDateTimeOffsetAddSeconds(const SystemCallInput& input, SystemCallResult* result) {
    dateTimeAddCore<DateTimeOffsetParts>([](auto* parts, auto count) { parts->addSeconds(count); }, input, result);
}

// (dto as DateTimeOffset, count as Number) as DateTimeOffset
void systemCallDateTimeOffsetAddMilliseconds(const SystemCallInput& input, SystemCallResult* result) {
    dateTimeAddCore<DateTimeOffsetParts>([](auto* parts, auto count) { parts->addMilliseconds(count); }, input, result);
}

// (dto as DateTime, count as Number) as DateTime
void systemCallDateTimeAddYears(const SystemCallInput& input, SystemCallResult* result) {
    dateTimeAddCore<DateTimeParts>([](auto* parts, auto count) { parts->addYears(count); }, input, result);
}

// (dto as DateTime, count as Number) as DateTime
void systemCallDateTimeAddMonths(const SystemCallInput& input, SystemCallResult* result) {
    dateTimeAddCore<DateTimeParts>([](auto* parts, auto count) { parts->addMonths(count); }, input, result);
}

// (dto as DateTime, count as Number) as DateTime
void systemCallDateTimeAddDays(const SystemCallInput& input, SystemCallResult* result) {
    dateTimeAddCore<DateTimeParts>([](auto* parts, auto count) { parts->addDays(count); }, input, result);
}

// (dto as DateTime, count as Number) as DateTime
void systemCallDateTimeAddHours(const SystemCallInput& input, SystemCallResult* result) {
    dateTimeAddCore<DateTimeParts>([](auto* parts, auto count) { parts->addHours(count); }, input, result);
}

// (dto as DateTime, count as Number) as DateTime
void systemCallDateTimeAddMinutes(const SystemCallInput& input, SystemCallResult* result) {
    dateTimeAddCore<DateTimeParts>([](auto* parts, auto count) { parts->addMinutes(count); }, input, result);
}

// (dto as DateTime, count as Number) as DateTime
void systemCallDateTimeAddSeconds(const SystemCallInput& input, SystemCallResult* result) {
    dateTimeAddCore<DateTimeParts>([](auto* parts, auto count) { parts->addSeconds(count); }, input, result);
}

// (dto as DateTime, count as Number) as DateTime
void systemCallDateTimeAddMilliseconds(const SystemCallInput& input, SystemCallResult* result) {
    dateTimeAddCore<DateTimeParts>([](auto* parts, auto count) { parts->addMilliseconds(count); }, input, result);
}

}  // namespace vm
