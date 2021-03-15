#include "systemCall.h"
#include "Error.h"
#include "List.h"
#include "Optional.h"
#include "String.h"
#include "TimeZone.h"
#include "date.h"
#include "util/decimal.h"

namespace vm {

typedef void (*SystemCallFunc)(const SystemCallInput&, SystemCallResult*);

static bool _systemCallsInitialized = false;
static std::vector<SystemCallFunc> _systemCalls;

SystemCallInput::SystemCallInput(
    const std::array<Value, kValueStackSize>& valueStack,
    const std::array<boost::local_shared_ptr<Object>, kObjectStackSize>& objectStack,
    int valueStackIndex,
    int objectStackIndex)
    : valueStack(valueStack),
      objectStack(objectStack),
      valueStackIndex(valueStackIndex),
      objectStackIndex(objectStackIndex) {}

const Value& SystemCallInput::getValue(const size_t index) const {
    return valueStack.at(valueStackIndex + index);
}

const Object& SystemCallInput::getObject(const size_t index) const {
    return *objectStack.at(objectStackIndex + index);
}

static void systemCallAvailableLocales(const SystemCallInput& /*unused*/, SystemCallResult* result) {
    int32_t count = 0;
    const auto* locales = icu::Locale::getAvailableLocales(count);

    auto objectListBuilder = ObjectListBuilder();
    for (int32_t i = 0; i < count; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto* name = locales[i].getName();
        objectListBuilder.items.push_back(boost::make_local_shared<String>(name, strlen(name)));
    }

    result->x = boost::make_local_shared<ObjectList>(&objectListBuilder);
}

static void systemCallAvailableTimeZones(const SystemCallInput& /*unused*/, SystemCallResult* result) {
    auto iter = std::unique_ptr<icu::StringEnumeration>(icu::TimeZone::createEnumeration());

    auto objectListBuilder = ObjectListBuilder();
    const char* item = nullptr;
    auto status = U_ZERO_ERROR;
    while ((item = iter->next(nullptr, status)) != nullptr) {
        objectListBuilder.items.push_back(boost::make_local_shared<String>(item, strlen(item)));
    }

    result->x = boost::make_local_shared<ObjectList>(&objectListBuilder);
}

static const icu::Locale& getBreakIteratorLocale(const icu::UnicodeString& name) {
    std::string nameUtf8;
    name.toUTF8String(nameUtf8);

    int32_t count = 0;
    const auto* locales = icu::BreakIterator::getAvailableLocales(count);
    for (int32_t i = 0; i < count; i++) {
        const auto& locale = locales[i];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (nameUtf8 == locale.getName()) {
            return locale;
        }
    }

    throw Error(ErrorCode::kInvalidLocaleName, "The locale name is invalid or unsupported.");
}

static void systemCallCharactersCore(
    const SystemCallInput& input,
    const icu::Locale& locale,
    SystemCallResult* result) {
    const auto& str = dynamic_cast<const String&>(input.getObject(0));
    auto status = U_ZERO_ERROR;
    auto breakIterator =
        std::unique_ptr<icu::BreakIterator>(icu::BreakIterator::createCharacterInstance(locale, status));
    breakIterator->setText(str.value);
    auto index1 = 0;
    auto index2 = breakIterator->first();
    auto objectListBuilder = ObjectListBuilder();
    while (index2 != icu::BreakIterator::DONE) {
        if (index2 > 0) {
            objectListBuilder.items.push_back(
                boost::make_local_shared<String>(icu::UnicodeString(str.value, index1, index2)));
        }

        index1 = index2;
        index2 = breakIterator->next();
    }

    result->x = boost::make_local_shared<ObjectList>(&objectListBuilder);
}

static void systemCallCharacters1(const SystemCallInput& input, SystemCallResult* result) {
    systemCallCharactersCore(input, icu::Locale::getUS(), result);
}

static void systemCallCharacters2(const SystemCallInput& input, SystemCallResult* result) {
    const auto& localeName = dynamic_cast<const String&>(input.getObject(1));
    const auto& locale = getBreakIteratorLocale(localeName.value);
    systemCallCharactersCore(input, locale, result);
}

static void systemCallChr(const SystemCallInput& input, SystemCallResult* result) {
    auto value = input.getValue(0).getInt64();
    auto ch = static_cast<UChar32>(value);
    result->x = boost::make_local_shared<String>(ch > 0 ? icu::UnicodeString(ch) : icu::UnicodeString());
}

static void systemCallDateFromParts(const SystemCallInput& input, SystemCallResult* result) {
    auto year = input.getValue(0).getInt32();
    auto month = input.getValue(1).getInt32();
    auto day = input.getValue(2).getInt32();
    result->a = newDate(year, month, day);
}

static void systemCallDateTimeFromParts(const SystemCallInput& input, SystemCallResult* result) {
    auto year = input.getValue(0).getInt32();
    auto month = input.getValue(1).getInt32();
    auto day = input.getValue(2).getInt32();
    auto hour = input.getValue(3).getInt32();
    auto minute = input.getValue(4).getInt32();
    auto second = input.getValue(5).getInt32();
    auto millisecond = input.getValue(6).getInt32();
    result->a = newDateTime(year, month, day, hour, minute, second, millisecond);
}

static void systemCallDateTimeOffsetFromParts(const SystemCallInput& input, SystemCallResult* result) {
    auto year = input.getValue(0).getInt32();
    auto month = input.getValue(1).getInt32();
    auto day = input.getValue(2).getInt32();
    auto hour = input.getValue(3).getInt32();
    auto minute = input.getValue(4).getInt32();
    auto second = input.getValue(5).getInt32();
    auto millisecond = input.getValue(6).getInt32();
    const auto& timeZone = dynamic_cast<const TimeZone&>(input.getObject(0));
    auto dateTime = newDateTime(year, month, day, hour, minute, second, millisecond);
    auto offset = Value(timeZone.getUtcOffset(dateTime.num));
    result->x = newDateTimeOffset(dateTime, offset);
}

static void systemCallDays(const SystemCallInput& input, SystemCallResult* result) {
    result->a.num = input.getValue(0).num * U_MILLIS_PER_DAY;
}

static void systemCallHasValueV(const SystemCallInput& input, SystemCallResult* result) {
    const auto& opt = dynamic_cast<const ValueOptional&>(input.getObject(0));
    result->a.setBoolean(opt.item.has_value());
}

static void systemCallHasValueO(const SystemCallInput& input, SystemCallResult* result) {
    const auto& opt = dynamic_cast<const ObjectOptional&>(input.getObject(0));
    result->a.setBoolean(opt.item.has_value());
}

static void systemCallHours(const SystemCallInput& input, SystemCallResult* result) {
    result->a.num = input.getValue(0).num * U_MILLIS_PER_HOUR;
}

static void systemCallLen(const SystemCallInput& input, SystemCallResult* result) {
    const auto& str = dynamic_cast<const String&>(input.getObject(0)).value;
    result->a.num = str.length();
}

static void systemCallMilliseconds(const SystemCallInput& input, SystemCallResult* result) {
    // already in milliseconds!
    result->a = input.getValue(0);
}

static void systemCallMinutes(const SystemCallInput& input, SystemCallResult* result) {
    result->a.num = input.getValue(0).num * U_MILLIS_PER_MINUTE;
}

static void systemCallSeconds(const SystemCallInput& input, SystemCallResult* result) {
    result->a.num = input.getValue(0).num * U_MILLIS_PER_SECOND;
}

static void systemCallTimeZoneFromName(const SystemCallInput& input, SystemCallResult* result) {
    const auto& name = dynamic_cast<const String&>(input.getObject(0));
    auto icuTimeZone = std::unique_ptr<icu::TimeZone>(icu::TimeZone::createTimeZone(name.value));
    icu::UnicodeString nameString;
    if (icuTimeZone->getID(nameString) == UCAL_UNKNOWN_ZONE_ID) {
        throw Error(ErrorCode::kInvalidTimeZone, "The specified time zone was not found.");
    }
    result->x = boost::make_local_shared<TimeZone>(std::move(icuTimeZone));
}

static void systemCallTotalDays(const SystemCallInput& input, SystemCallResult* result) {
    result->a.num = input.getValue(0).num / U_MILLIS_PER_DAY;
}

static void systemCallTotalHours(const SystemCallInput& input, SystemCallResult* result) {
    result->a.num = input.getValue(0).num / U_MILLIS_PER_HOUR;
}

static void systemCallTotalMilliseconds(const SystemCallInput& input, SystemCallResult* result) {
    // already in milliseconds!
    result->a = input.getValue(0);
}

static void systemCallTotalMinutes(const SystemCallInput& input, SystemCallResult* result) {
    result->a.num = input.getValue(0).num / U_MILLIS_PER_MINUTE;
}

static void systemCallTotalSeconds(const SystemCallInput& input, SystemCallResult* result) {
    result->a.num = input.getValue(0).num / U_MILLIS_PER_SECOND;
}

static void systemCallUtcOffset(const SystemCallInput& input, SystemCallResult* result) {
    const auto& timeZone = dynamic_cast<const TimeZone&>(input.getObject(0));
    const auto& dateTime = input.getValue(0);
    result->a.num = timeZone.getUtcOffset(dateTime.num);
}

static void systemCallValueV(const SystemCallInput& input, SystemCallResult* result) {
    const auto& opt = dynamic_cast<const ValueOptional&>(input.getObject(0));
    if (!opt.item.has_value()) {
        throw Error(ErrorCode::kValueNotPresent, "Optional value is not present.");
    }
    result->a = *opt.item;
}

static void systemCallValueO(const SystemCallInput& input, SystemCallResult* result) {
    const auto& opt = dynamic_cast<const ObjectOptional&>(input.getObject(0));
    if (!opt.item.has_value()) {
        throw Error(ErrorCode::kValueNotPresent, "Optional value is not present.");
    }
    result->x = *opt.item;
}

static void initSystemCall(SystemCall which, SystemCallFunc func) {
    auto index = static_cast<size_t>(which);

    while (_systemCalls.size() <= index) {
        _systemCalls.push_back(nullptr);
    }

    _systemCalls.at(index) = func;
}

void initSystemCalls() {
    if (_systemCallsInitialized) {
        return;
    }

    initSystemCall(SystemCall::kAvailableLocales, systemCallAvailableLocales);
    initSystemCall(SystemCall::kAvailableTimeZones, systemCallAvailableTimeZones);
    initSystemCall(SystemCall::kCharacters1, systemCallCharacters1);
    initSystemCall(SystemCall::kCharacters2, systemCallCharacters2);
    initSystemCall(SystemCall::kChr, systemCallChr);
    initSystemCall(SystemCall::kDateFromParts, systemCallDateFromParts);
    initSystemCall(SystemCall::kDateTimeFromParts, systemCallDateTimeFromParts);
    initSystemCall(SystemCall::kDateTimeOffsetFromParts, systemCallDateTimeOffsetFromParts);
    initSystemCall(SystemCall::kDays, systemCallDays);
    initSystemCall(SystemCall::kHasValueO, systemCallHasValueO);
    initSystemCall(SystemCall::kHasValueV, systemCallHasValueV);
    initSystemCall(SystemCall::kHours, systemCallHours);
    initSystemCall(SystemCall::kLen, systemCallLen);
    initSystemCall(SystemCall::kMilliseconds, systemCallMilliseconds);
    initSystemCall(SystemCall::kMinutes, systemCallMinutes);
    initSystemCall(SystemCall::kSeconds, systemCallSeconds);
    initSystemCall(SystemCall::kTimeZoneFromName, systemCallTimeZoneFromName);
    initSystemCall(SystemCall::kTotalDays, systemCallTotalDays);
    initSystemCall(SystemCall::kTotalHours, systemCallTotalHours);
    initSystemCall(SystemCall::kTotalMilliseconds, systemCallTotalMilliseconds);
    initSystemCall(SystemCall::kTotalMinutes, systemCallTotalMinutes);
    initSystemCall(SystemCall::kTotalSeconds, systemCallTotalSeconds);
    initSystemCall(SystemCall::kUtcOffset, systemCallUtcOffset);
    initSystemCall(SystemCall::kValueO, systemCallValueO);
    initSystemCall(SystemCall::kValueV, systemCallValueV);

    _systemCallsInitialized = true;
}

SystemCallResult systemCall(SystemCall which, const SystemCallInput& input) {
    SystemCallResult result;

    try {
        auto index = static_cast<size_t>(which);
        _systemCalls.at(index)(input, &result);
    } catch (Error& ex) {
        result.hasError = true;
        result.errorCode = static_cast<int>(ex.code);
        result.errorMessage = ex.what();
    } catch (std::exception& ex) {
        result.hasError = true;
        result.errorCode = -1;
        result.errorMessage = ex.what();
    }

    return result;
}

}  // namespace vm
