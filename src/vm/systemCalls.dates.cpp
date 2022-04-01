#include "systemCall.h"
#include "Error.h"
#include "List.h"
#include "String.h"
#include "TimeZone.h"
#include "date.h"

namespace vm {

static void systemCallAvailableTimeZones(const SystemCallInput& /*unused*/, SystemCallResult* result) {
    auto status = U_ZERO_ERROR;
    auto iter = std::unique_ptr<icu::StringEnumeration>(icu::TimeZone::createEnumeration(status));
    if (U_FAILURE(status)) {
        throw Error(ErrorCode::kInternalIcuError, "Failed to retrieve the time zone list from ICU.");
    }

    auto objectListBuilder = ObjectListBuilder();
    const char* item = nullptr;
    while ((item = iter->next(nullptr, status)) != nullptr) {
        objectListBuilder.items.push_back(boost::make_local_shared<String>(item, strlen(item)));
    }

    result->returnedObject = boost::make_local_shared<ObjectList>(&objectListBuilder);
}

void initSystemCallsDates() {
    initSystemCall(SystemCall::kAvailableTimeZones, systemCallAvailableTimeZones);

    initSystemCall(SystemCall::kDateFromParts, [](const auto& input, auto* result) {
        auto year = input.getValue(-3).getInt32();
        auto month = input.getValue(-2).getInt32();
        auto day = input.getValue(-1).getInt32();
        result->returnedValue = newDate(year, month, day);
    });

    initSystemCall(SystemCall::kDateTimeFromParts, [](const auto& input, auto* result) {
        auto year = input.getValue(-7).getInt32();
        auto month = input.getValue(-6).getInt32();
        auto day = input.getValue(-5).getInt32();
        auto hour = input.getValue(-4).getInt32();
        auto minute = input.getValue(-3).getInt32();
        auto second = input.getValue(-2).getInt32();
        auto millisecond = input.getValue(-1).getInt32();
        result->returnedValue = newDateTime(year, month, day, hour, minute, second, millisecond);
    });

    initSystemCall(SystemCall::kDateTimeOffsetFromParts, [](const auto& input, auto* result) {
        auto year = input.getValue(-7).getInt32();
        auto month = input.getValue(-6).getInt32();
        auto day = input.getValue(-5).getInt32();
        auto hour = input.getValue(-4).getInt32();
        auto minute = input.getValue(-3).getInt32();
        auto second = input.getValue(-2).getInt32();
        auto millisecond = input.getValue(-1).getInt32();
        const auto& timeZone = dynamic_cast<const TimeZone&>(input.getObject(-1));
        auto dateTime = newDateTime(year, month, day, hour, minute, second, millisecond);
        Value offset{ timeZone.getUtcOffset(dateTime.num) };
        result->returnedObject = newDateTimeOffset(dateTime, offset);
    });

    initSystemCall(SystemCall::kDateTimeToDate, [](const auto& input, auto* result) {
        const auto& dateTime = input.getValue(-1);
        result->returnedValue = dateTimeToDate(dateTime);
    });

    initSystemCall(SystemCall::kDateToString, [](const auto& input, auto* result) {
        const auto& date = input.getValue(-1);
        result->returnedObject = dateToString(date);
    });

    initSystemCall(SystemCall::kDateTimeToString, [](const auto& input, auto* result) {
        const auto& dateTime = input.getValue(-1);
        result->returnedObject = dateTimeToString(dateTime);
    });

    initSystemCall(SystemCall::kDateTimeOffsetToString, [](const auto& input, auto* result) {
        const auto& date = dynamic_cast<const Record&>(input.getObject(-1));
        result->returnedObject = dateTimeOffsetToString(date);
    });

    initSystemCall(SystemCall::kDays, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num * U_MILLIS_PER_DAY;
    });

    initSystemCall(SystemCall::kHours, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num * U_MILLIS_PER_HOUR;
    });

    initSystemCall(SystemCall::kMilliseconds, [](const auto& input, auto* result) {
        result->returnedValue = input.getValue(-1);  // already in milliseconds!
    });

    initSystemCall(SystemCall::kMinutes, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num * U_MILLIS_PER_MINUTE;
    });

    initSystemCall(SystemCall::kSeconds, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num * U_MILLIS_PER_SECOND;
    });

    initSystemCall(SystemCall::kTimeSpanToString, [](const auto& input, auto* result) {
        result->returnedObject = timeSpanToString(input.getValue(-1));
    });

    initSystemCall(SystemCall::kTimeZoneToString, [](const auto& input, auto* result) {
        const auto& timeZone = dynamic_cast<const TimeZone&>(input.getObject(-1));
        icu::UnicodeString name{};
        timeZone.zone->getDisplayName(name);
        result->returnedObject = boost::make_local_shared<String>(std::move(name));
    });

    initSystemCall(SystemCall::kTimeZoneFromName, [](const auto& input, auto* result) {
        const auto& name = dynamic_cast<const String&>(input.getObject(-1));
        auto icuTimeZone = std::unique_ptr<icu::TimeZone>(icu::TimeZone::createTimeZone(name.value));
        icu::UnicodeString nameString;
        if ((icuTimeZone->getID(nameString) == UCAL_UNKNOWN_ZONE_ID) != 0) {
            throw Error(ErrorCode::kInvalidTimeZone, "The specified time zone was not found.");
        }
        result->returnedObject = boost::make_local_shared<TimeZone>(std::move(icuTimeZone));
    });

    initSystemCall(SystemCall::kTotalDays, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num / U_MILLIS_PER_DAY;
    });

    initSystemCall(SystemCall::kTotalHours, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num / U_MILLIS_PER_HOUR;
    });

    initSystemCall(SystemCall::kTotalMilliseconds, [](const auto& input, auto* result) {
        result->returnedValue = input.getValue(-1);  // already in milliseconds!
    });

    initSystemCall(SystemCall::kTotalMinutes, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num / U_MILLIS_PER_MINUTE;
    });

    initSystemCall(SystemCall::kTotalSeconds, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num / U_MILLIS_PER_SECOND;
    });

    initSystemCall(SystemCall::kUtcOffset, [](const auto& input, auto* result) {
        const auto& timeZone = dynamic_cast<const TimeZone&>(input.getObject(-1));
        const auto& dateTime = input.getValue(-1);
        result->returnedValue.num = timeZone.getUtcOffset(dateTime.num);
    });
}

}  // namespace vm
