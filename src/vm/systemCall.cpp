#include "systemCall.h"
#include "Error.h"
#include "List.h"
#include "Map.h"
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
    int objectStackIndex,
    std::istream* consoleInputStream,
    std::ostream* consoleOutputStream)
    : valueStack(valueStack),
      objectStack(objectStack),
      valueStackIndex(valueStackIndex),
      objectStackIndex(objectStackIndex),
      consoleInputStream(consoleInputStream),
      consoleOutputStream(consoleOutputStream) {}

const Value& SystemCallInput::getValue(const int vsiOffset) const {
    return valueStack.at(valueStackIndex + vsiOffset);
}

const Object& SystemCallInput::getObject(const int osiOffset) const {
    return *objectStack.at(objectStackIndex + osiOffset);
}

boost::local_shared_ptr<Object> SystemCallInput::getObjectPtr(const int osiOffset) const {
    return objectStack.at(objectStackIndex + osiOffset);
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

    result->returnedObject = boost::make_local_shared<ObjectList>(&objectListBuilder);
}

static void systemCallAvailableTimeZones(const SystemCallInput& /*unused*/, SystemCallResult* result) {
    auto iter = std::unique_ptr<icu::StringEnumeration>(icu::TimeZone::createEnumeration());

    auto objectListBuilder = ObjectListBuilder();
    const char* item = nullptr;
    auto status = U_ZERO_ERROR;
    while ((item = iter->next(nullptr, status)) != nullptr) {
        objectListBuilder.items.push_back(boost::make_local_shared<String>(item, strlen(item)));
    }

    result->returnedObject = boost::make_local_shared<ObjectList>(&objectListBuilder);
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
    const auto& str = dynamic_cast<const String&>(input.getObject(-1));
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

    result->returnedObject = boost::make_local_shared<ObjectList>(&objectListBuilder);
}

static void systemCallCharacters1(const SystemCallInput& input, SystemCallResult* result) {
    systemCallCharactersCore(input, icu::Locale::getUS(), result);
}

static void systemCallCharacters2(const SystemCallInput& input, SystemCallResult* result) {
    const auto& localeName = dynamic_cast<const String&>(input.getObject(-2));
    const auto& locale = getBreakIteratorLocale(localeName.value);
    systemCallCharactersCore(input, locale, result);
}

static void systemCallChr(const SystemCallInput& input, SystemCallResult* result) {
    auto value = input.getValue(-1).getInt64();
    auto ch = static_cast<UChar32>(value);
    result->returnedObject = boost::make_local_shared<String>(ch > 0 ? icu::UnicodeString(ch) : icu::UnicodeString());
}

static void systemCallCounterIsPastLimit(const SystemCallInput& input, SystemCallResult* result) {
    // used with 'for' loops
    const auto& counter = input.getValue(-3).num;
    const auto& limit = input.getValue(-2).num;
    const auto& step = input.getValue(-1).num;
    bool condition{};
    if (step >= 0) {
        condition = counter > limit;
    } else {
        condition = counter < limit;
    }
    result->returnedValue.setBoolean(condition);
}

static void systemCallDateFromParts(const SystemCallInput& input, SystemCallResult* result) {
    auto year = input.getValue(-3).getInt32();
    auto month = input.getValue(-2).getInt32();
    auto day = input.getValue(-1).getInt32();
    result->returnedValue = newDate(year, month, day);
}

static void systemCallDateTimeFromParts(const SystemCallInput& input, SystemCallResult* result) {
    auto year = input.getValue(-7).getInt32();
    auto month = input.getValue(-6).getInt32();
    auto day = input.getValue(-5).getInt32();
    auto hour = input.getValue(-4).getInt32();
    auto minute = input.getValue(-3).getInt32();
    auto second = input.getValue(-2).getInt32();
    auto millisecond = input.getValue(-1).getInt32();
    result->returnedValue = newDateTime(year, month, day, hour, minute, second, millisecond);
}

static void systemCallDateTimeOffsetFromParts(const SystemCallInput& input, SystemCallResult* result) {
    auto year = input.getValue(-7).getInt32();
    auto month = input.getValue(-6).getInt32();
    auto day = input.getValue(-5).getInt32();
    auto hour = input.getValue(-4).getInt32();
    auto minute = input.getValue(-3).getInt32();
    auto second = input.getValue(-2).getInt32();
    auto millisecond = input.getValue(-1).getInt32();
    const auto& timeZone = dynamic_cast<const TimeZone&>(input.getObject(-1));
    auto dateTime = newDateTime(year, month, day, hour, minute, second, millisecond);
    auto offset = Value(timeZone.getUtcOffset(dateTime.num));
    result->returnedObject = newDateTimeOffset(dateTime, offset);
}

static void systemCallHasValueV(const SystemCallInput& input, SystemCallResult* result) {
    const auto& opt = dynamic_cast<const ValueOptional&>(input.getObject(-1));
    result->returnedValue.setBoolean(opt.item.has_value());
}

static void systemCallHasValueO(const SystemCallInput& input, SystemCallResult* result) {
    const auto& opt = dynamic_cast<const ObjectOptional&>(input.getObject(-1));
    result->returnedValue.setBoolean(opt.item.has_value());
}

static void systemCallHours(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num * U_MILLIS_PER_HOUR;
}

static void systemCallInputString(const SystemCallInput& input, SystemCallResult* result) {
    std::string line;
    std::getline(*input.consoleInputStream, line);
    result->returnedObject = boost::make_local_shared<String>(line);
}

static void systemCallLen(const SystemCallInput& input, SystemCallResult* result) {
    const auto& str = dynamic_cast<const String&>(input.getObject(-1)).value;
    result->returnedValue.num = str.length();
}

static void systemCallNumberToString(const SystemCallInput& input, SystemCallResult* result) {
    const auto& value = input.getValue(-1);
    result->returnedObject = boost::make_local_shared<String>(value.getString());
}

static void systemCallObjectListGet(const SystemCallInput& input, SystemCallResult* result) {
    const auto& objectList = dynamic_cast<const ObjectList&>(input.getObject(-1));
    const auto& index = input.getValue(-1).getInt64();
    result->returnedObject = objectList.items.at(index);
}

static void systemCallObjectListLength(const SystemCallInput& input, SystemCallResult* result) {
    const auto& objectList = dynamic_cast<const ObjectList&>(input.getObject(-1));
    result->returnedValue.num = objectList.items.size();
}

static void systemCallTimeZoneFromName(const SystemCallInput& input, SystemCallResult* result) {
    const auto& name = dynamic_cast<const String&>(input.getObject(-1));
    auto icuTimeZone = std::unique_ptr<icu::TimeZone>(icu::TimeZone::createTimeZone(name.value));
    icu::UnicodeString nameString;
    if ((icuTimeZone->getID(nameString) == UCAL_UNKNOWN_ZONE_ID) != 0) {
        throw Error(ErrorCode::kInvalidTimeZone, "The specified time zone was not found.");
    }
    result->returnedObject = boost::make_local_shared<TimeZone>(std::move(icuTimeZone));
}

static void systemCallUtcOffset(const SystemCallInput& input, SystemCallResult* result) {
    const auto& timeZone = dynamic_cast<const TimeZone&>(input.getObject(-1));
    const auto& dateTime = input.getValue(-1);
    result->returnedValue.num = timeZone.getUtcOffset(dateTime.num);
}

static void systemCallValueV(const SystemCallInput& input, SystemCallResult* result) {
    const auto& opt = dynamic_cast<const ValueOptional&>(input.getObject(-1));
    if (!opt.item.has_value()) {
        throw Error(ErrorCode::kValueNotPresent, "Optional value is not present.");
    }
    result->returnedValue = *opt.item;
}

static void systemCallValueListGet(const SystemCallInput& input, SystemCallResult* result) {
    const auto& valueList = dynamic_cast<const ValueList&>(input.getObject(-1));
    const auto& index = input.getValue(-1).getInt64();
    result->returnedValue = valueList.items.at(index);
}

static void systemCallValueO(const SystemCallInput& input, SystemCallResult* result) {
    const auto& opt = dynamic_cast<const ObjectOptional&>(input.getObject(-1));
    if (!opt.item.has_value()) {
        throw Error(ErrorCode::kValueNotPresent, "Optional value is not present.");
    }
    result->returnedObject = *opt.item;
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
    initSystemCall(SystemCall::kCounterIsPastLimit, systemCallCounterIsPastLimit);
    initSystemCall(SystemCall::kDateFromParts, systemCallDateFromParts);
    initSystemCall(SystemCall::kDateTimeFromParts, systemCallDateTimeFromParts);
    initSystemCall(SystemCall::kDateTimeOffsetFromParts, systemCallDateTimeOffsetFromParts);
    initSystemCall(SystemCall::kDays, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num * U_MILLIS_PER_DAY;
    });
    initSystemCall(
        SystemCall::kFlushConsoleOutput, [](const auto& input, auto* result) { input.consoleOutputStream->flush(); });
    initSystemCall(SystemCall::kHasValueO, systemCallHasValueO);
    initSystemCall(SystemCall::kHasValueV, systemCallHasValueV);
    initSystemCall(SystemCall::kHours, systemCallHours);
    initSystemCall(SystemCall::kInputString, systemCallInputString);
    initSystemCall(SystemCall::kLen, systemCallLen);
    initSystemCall(SystemCall::kMilliseconds, [](const auto& input, auto* result) {
        result->returnedValue = input.getValue(-1);  // already in milliseconds!
    });
    initSystemCall(SystemCall::kMinutes, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num * U_MILLIS_PER_MINUTE;
    });
    initSystemCall(SystemCall::kNumberAdd, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num + input.getValue(-1).num;
    });
    initSystemCall(SystemCall::kNumberDivide, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num / input.getValue(-1).num;
    });
    initSystemCall(SystemCall::kNumberEquals, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num == input.getValue(-1).num ? 1 : 0;
    });
    initSystemCall(SystemCall::kNumberGreaterThan, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num > input.getValue(-1).num ? 1 : 0;
    });
    initSystemCall(SystemCall::kNumberGreaterThanEquals, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num >= input.getValue(-1).num ? 1 : 0;
    });
    initSystemCall(SystemCall::kNumberLessThan, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num < input.getValue(-1).num ? 1 : 0;
    });
    initSystemCall(SystemCall::kNumberLessThanEquals, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num <= input.getValue(-1).num ? 1 : 0;
    });
    initSystemCall(SystemCall::kNumberModulus, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num % input.getValue(-1).num;
    });
    initSystemCall(SystemCall::kNumberMultiply, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num * input.getValue(-1).num;
    });
    initSystemCall(SystemCall::kNumberNotEquals, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num != input.getValue(-1).num ? 1 : 0;
    });
    initSystemCall(SystemCall::kNumberSubtract, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num - input.getValue(-1).num;
    });
    initSystemCall(SystemCall::kNumberToString, systemCallNumberToString);
    initSystemCall(SystemCall::kObjectListGet, systemCallObjectListGet);
    initSystemCall(SystemCall::kObjectListLength, systemCallObjectListLength);
    initSystemCall(SystemCall::kObjectOptionalNewMissing, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ObjectOptional>();
    });
    initSystemCall(SystemCall::kObjectOptionalNewPresent, [](const auto& input, auto* result) {
        result->returnedObject = boost::make_local_shared<ObjectOptional>(input.getObjectPtr(-1));
    });
    initSystemCall(SystemCall::kObjectToObjectMapNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ObjectToObjectMap>();
    });
    initSystemCall(SystemCall::kObjectToValueMapNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ObjectToValueMap>();
    });
    initSystemCall(SystemCall::kPrintString, [](const auto& input, auto* result) {
        *input.consoleOutputStream << dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
    });
    initSystemCall(SystemCall::kSeconds, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num * U_MILLIS_PER_SECOND;
    });
    initSystemCall(SystemCall::kTimeZoneFromName, systemCallTimeZoneFromName);
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
    initSystemCall(SystemCall::kUtcOffset, systemCallUtcOffset);
    initSystemCall(SystemCall::kValueO, systemCallValueO);
    initSystemCall(SystemCall::kValueListGet, systemCallValueListGet);
    initSystemCall(SystemCall::kValueOptionalNewMissing, [](const auto& input, auto* result) {
        result->returnedObject = boost::make_local_shared<ValueOptional>();
    });
    initSystemCall(SystemCall::kValueOptionalNewPresent, [](const auto& input, auto* result) {
        result->returnedObject = boost::make_local_shared<ValueOptional>(input.getValue(-1));
    });
    initSystemCall(SystemCall::kValueToObjectMapNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ValueToObjectMap>();
    });
    initSystemCall(SystemCall::kValueToValueMapNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ValueToValueMap>();
    });
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
