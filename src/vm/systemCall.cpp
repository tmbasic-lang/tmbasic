#include "systemCall.h"
#include "Error.h"
#include "List.h"
#include "Map.h"
#include "Optional.h"
#include "String.h"
#include "TimeZone.h"
#include "constants.h"
#include "date.h"
#include "filesystem.h"
#include "util/decimal.h"
#include "util/path.h"

namespace vm {

typedef void (*SystemCallFunc)(const SystemCallInput&, SystemCallResult*);

static bool _systemCallsInitialized = false;
static std::vector<SystemCallFunc> _systemCalls;

SystemCallInput::SystemCallInput(
    std::array<Value, kValueStackSize>* valueStack,
    std::array<boost::local_shared_ptr<Object>, kObjectStackSize>* objectStack,
    int valueStackIndex,
    int objectStackIndex,
    std::istream* consoleInputStream,
    std::ostream* consoleOutputStream,
    const Value& errorCode,
    const std::string& errorMessage)
    : valueStack(valueStack),
      objectStack(objectStack),
      valueStackIndex(valueStackIndex),
      objectStackIndex(objectStackIndex),
      consoleInputStream(consoleInputStream),
      consoleOutputStream(consoleOutputStream),
      errorCode(errorCode),
      errorMessage(errorMessage) {}

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
    Value offset{ timeZone.getUtcOffset(dateTime.num) };
    result->returnedObject = newDateTimeOffset(dateTime, offset);
}

static void systemCallDateToString(const SystemCallInput& input, SystemCallResult* result) {
    const auto& date = input.getValue(-1);
    result->returnedObject = dateToString(date);
}

static void systemCallDateTimeToString(const SystemCallInput& input, SystemCallResult* result) {
    const auto& dateTime = input.getValue(-1);
    result->returnedObject = dateTimeToString(dateTime);
}

static void systemCallDateTimeOffsetToString(const SystemCallInput& input, SystemCallResult* result) {
    const auto& date = dynamic_cast<const Record&>(input.getObject(-1));
    result->returnedObject = dateTimeOffsetToString(date);
}

static void systemCallHasValue(const SystemCallInput& input, SystemCallResult* result) {
    auto& object = input.getObject(-1);
    const auto* valueOptional = dynamic_cast<const ValueOptional*>(&object);
    if (valueOptional != nullptr) {
        result->returnedValue.setBoolean(valueOptional->item.has_value());
        return;
    }

    const auto* objectOptional = dynamic_cast<const ObjectOptional*>(&object);
    if (objectOptional != nullptr) {
        result->returnedValue.setBoolean(objectOptional->item.has_value());
        return;
    }

    throw Error(
        ErrorCode::kInternalTypeConfusion,
        fmt::format(
            "Internal type confusion error. HasValue target is neither {} nor {}.", NAMEOF_TYPE(ValueOptional),
            NAMEOF_TYPE(ObjectOptional)));
}

static void systemCallHours(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num * U_MILLIS_PER_HOUR;
}

static void systemCallInputString(const SystemCallInput& input, SystemCallResult* result) {
    std::string line;
    std::getline(*input.consoleInputStream, line);
    result->returnedObject = boost::make_local_shared<String>(line);
}

static void systemCallStringLen(const SystemCallInput& input, SystemCallResult* result) {
    const auto& str = dynamic_cast<const String&>(input.getObject(-1)).value;
    result->returnedValue.num = str.length();
}

static void systemCallNumberToString(const SystemCallInput& input, SystemCallResult* result) {
    const auto& value = input.getValue(-1);
    result->returnedObject = boost::make_local_shared<String>(value.getString());
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

static void systemCallValue(const SystemCallInput& input, SystemCallResult* result) {
    auto& object = input.getObject(-1);
    const auto* valueOptional = dynamic_cast<const ValueOptional*>(&object);
    if (valueOptional != nullptr) {
        if (!valueOptional->item.has_value()) {
            throw Error(ErrorCode::kValueNotPresent, "Optional value is not present.");
        }
        result->returnedValue = *valueOptional->item;
        return;
    }

    const auto* objectOptional = dynamic_cast<const ObjectOptional*>(&object);
    if (objectOptional != nullptr) {
        if (!objectOptional->item.has_value()) {
            throw Error(ErrorCode::kValueNotPresent, "Optional value is not present.");
        }
        result->returnedObject = *objectOptional->item;
        return;
    }

    throw Error(
        ErrorCode::kInternalTypeConfusion,
        fmt::format(
            "Internal type confusion error. HasValue target is neither {} nor {}.", NAMEOF_TYPE(ValueOptional),
            NAMEOF_TYPE(ObjectOptional)));
}

static void systemCallListFirstOrLast(const SystemCallInput& input, SystemCallResult* result, bool first) {
    auto& object = input.getObject(-1);
    const auto* valueList = dynamic_cast<const ValueList*>(&object);
    if (valueList != nullptr) {
        if (valueList->items.empty()) {
            throw Error(ErrorCode::kListIsEmpty, "List is empty.");
        }
        result->returnedValue = first ? valueList->items.front() : valueList->items.back();
        return;
    }

    const auto* objectList = dynamic_cast<const ObjectList*>(&object);
    if (objectList != nullptr) {
        if (objectList->items.empty()) {
            throw Error(ErrorCode::kListIsEmpty, "List is empty.");
        }
        result->returnedObject = first ? objectList->items.front() : objectList->items.back();
        return;
    }

    throw Error(
        ErrorCode::kInternalTypeConfusion,
        fmt::format(
            "Internal type confusion error. ListFirst target is neither {} nor {}.", NAMEOF_TYPE(ValueList),
            NAMEOF_TYPE(ObjectList)));
}

static boost::local_shared_ptr<String> stringConcat(const ObjectList& objectList, const String& separator) {
    std::vector<char16_t> uchars{};
    bool isFirst = true;
    size_t numSepCodeUnits = separator.value.length();
    for (const auto& object : objectList.items) {
        if (!isFirst && numSepCodeUnits > 0) {
            uchars.reserve(uchars.size() + numSepCodeUnits);
            for (size_t i = 0; i < numSepCodeUnits; i++) {
                uchars.push_back(separator.value.charAt(i));
            }
        }
        isFirst = false;

        const auto& str = dynamic_cast<const String&>(*object);
        size_t numCodeUnits = str.value.length();
        uchars.reserve(uchars.size() + numCodeUnits);
        for (size_t i = 0; i < numCodeUnits; i++) {
            uchars.push_back(str.value.charAt(i));
        }
    }
    icu::UnicodeString str{ uchars.data(), static_cast<int32_t>(uchars.size()) };
    return boost::make_local_shared<String>(std::move(str));
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

    initSystemCall(SystemCall::kAbs, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num.abs();
    });
    initSystemCall(SystemCall::kAcos, [](const auto& input, auto* result) {
        result->returnedValue.setDouble(std::acos(input.getValue(-1).getDouble()));
    });
    initSystemCall(SystemCall::kAsin, [](const auto& input, auto* result) {
        result->returnedValue.setDouble(std::asin(input.getValue(-1).getDouble()));
    });
    initSystemCall(SystemCall::kAtan, [](const auto& input, auto* result) {
        result->returnedValue.setDouble(std::atan(input.getValue(-1).getDouble()));
    });
    initSystemCall(SystemCall::kAtan2, [](const auto& input, auto* result) {
        result->returnedValue.setDouble(std::atan2(input.getValue(-2).getDouble(), input.getValue(-1).getDouble()));
    });
    initSystemCall(SystemCall::kAvailableLocales, systemCallAvailableLocales);
    initSystemCall(SystemCall::kAvailableTimeZones, systemCallAvailableTimeZones);
    initSystemCall(SystemCall::kBooleanAnd, [](const auto& input, auto* result) {
        result->returnedValue.setBoolean(input.getValue(-2).getBoolean() && input.getValue(-1).getBoolean());
    });
    initSystemCall(SystemCall::kBooleanNot, [](const auto& input, auto* result) {
        result->returnedValue.setBoolean(!input.getValue(-1).getBoolean());
    });
    initSystemCall(SystemCall::kBooleanOr, [](const auto& input, auto* result) {
        result->returnedValue.setBoolean(input.getValue(-2).getBoolean() || input.getValue(-1).getBoolean());
    });
    initSystemCall(SystemCall::kBooleanToString, [](const auto& input, auto* result) {
        std::string s{ input.getValue(-1).getBoolean() ? "true" : "false" };
        result->returnedObject = boost::make_local_shared<String>(std::move(s));
    });
    initSystemCall(SystemCall::kCeil, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num.ceil();
    });
    initSystemCall(SystemCall::kCharacters1, systemCallCharacters1);
    initSystemCall(SystemCall::kCharacters2, systemCallCharacters2);
    initSystemCall(SystemCall::kChr, systemCallChr);
    initSystemCall(SystemCall::kCodePoints, [](const auto& input, auto* result) {
        const auto& str = dynamic_cast<const String&>(input.getObject(-1));
        auto numCodePoints = str.value.countChar32();
        std::vector<int32_t> codePoints(numCodePoints);
        UErrorCode status = U_ZERO_ERROR;
        str.value.toUTF32(codePoints.data(), numCodePoints, status);
        if (U_FAILURE(status)) {
            throw Error(
                ErrorCode::kInternalIcuError,
                fmt::format("Failed to convert the string to code points. ICU error: {}", u_errorName(status)));
        }
        ValueListBuilder vlb{};
        for (const auto& codePoint : codePoints) {
            vlb.items.push_back(Value{ codePoint });
        }
        result->returnedObject = boost::make_local_shared<ValueList>(&vlb);
    });
    initSystemCall(SystemCall::kCodeUnit1, [](const auto& input, auto* result) {
        const auto& str = dynamic_cast<const String&>(input.getObject(-1));
        auto codeUnit = str.value.charAt(0);
        result->returnedValue.num = codeUnit == 0xFFFF ? 0 : codeUnit;
    });
    initSystemCall(SystemCall::kCodeUnit2, [](const auto& input, auto* result) {
        const auto& str = dynamic_cast<const String&>(input.getObject(-1));
        const auto& index = input.getValue(-1).getInt64();
        auto codeUnit = str.value.charAt(index);
        result->returnedValue.num = codeUnit == 0xFFFF ? 0 : codeUnit;
    });
    initSystemCall(SystemCall::kCodeUnits, [](const auto& input, auto* result) {
        const auto& str = dynamic_cast<const String&>(input.getObject(-1));
        ValueListBuilder b{};
        size_t numCodeUnits = str.value.length();
        for (size_t i = 0; i < numCodeUnits; i++) {
            b.items.push_back(Value{ static_cast<int64_t>(str.value.charAt(i)) });
        }
        result->returnedObject = boost::make_local_shared<ValueList>(&b);
    });
    initSystemCall(SystemCall::kConcat1, [](const auto& input, auto* result) {
        const auto& objectList = dynamic_cast<const ObjectList&>(input.getObject(-1));
        String empty{ "", 0 };
        result->returnedObject = stringConcat(objectList, empty);
    });
    initSystemCall(SystemCall::kConcat2, [](const auto& input, auto* result) {
        const auto& objectList = dynamic_cast<const ObjectList&>(input.getObject(-2));
        const auto& separator = dynamic_cast<const String&>(input.getObject(-1));
        result->returnedObject = stringConcat(objectList, separator);
    });
    initSystemCall(SystemCall::kCos, [](const auto& input, auto* result) {
        result->returnedValue.setDouble(std::cos(input.getValue(-1).getDouble()));
    });
    initSystemCall(SystemCall::kCounterIsPastLimit, systemCallCounterIsPastLimit);
    initSystemCall(SystemCall::kCreateDirectory, [](const auto& input, auto* /*result*/) {
        const auto& path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        vm::createDirectory(path);
    });
    initSystemCall(SystemCall::kDateFromParts, systemCallDateFromParts);
    initSystemCall(SystemCall::kDateTimeFromParts, systemCallDateTimeFromParts);
    initSystemCall(SystemCall::kDateTimeOffsetFromParts, systemCallDateTimeOffsetFromParts);
    initSystemCall(SystemCall::kDateToString, systemCallDateToString);
    initSystemCall(SystemCall::kDateTimeToDate, [](const auto& input, auto* result) {
        const auto& dateTime = input.getValue(-1);
        result->returnedValue = dateTimeToDate(dateTime);
    });
    initSystemCall(SystemCall::kDateTimeToString, systemCallDateTimeToString);
    initSystemCall(SystemCall::kDateTimeOffsetToString, systemCallDateTimeOffsetToString);
    initSystemCall(SystemCall::kDays, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num * U_MILLIS_PER_DAY;
    });
    initSystemCall(SystemCall::kDeleteDirectory1, [](const auto& input, auto* /*result*/) {
        const auto& path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        vm::deleteDirectory(path, false);
    });
    initSystemCall(SystemCall::kDeleteDirectory2, [](const auto& input, auto* /*result*/) {
        const auto& path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        auto recursive = input.getValue(-1).getBoolean();
        vm::deleteDirectory(path, recursive);
    });
    initSystemCall(SystemCall::kDeleteFile, [](const auto& input, auto* /*result*/) {
        const auto& path = dynamic_cast<const String&>(input.getObject(-1));
        auto pathStr = path.toUtf8();
        if (unlink(pathStr.c_str()) != 0) {
            auto err = errno;
            if (err == ENOENT) {
                // not an error
                return;
            }
            throw Error::fromFileErrno(err, pathStr);
        }
    });
    initSystemCall(
        SystemCall::kErrorCode, [](const auto& input, auto* result) { result->returnedValue = input.errorCode; });
    initSystemCall(SystemCall::kErrorMessage, [](const auto& input, auto* result) {
        result->returnedObject = boost::make_local_shared<String>(input.errorMessage);
    });
    initSystemCall(SystemCall::kExp, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num.exp();
    });
    initSystemCall(SystemCall::kFileExists, [](const auto& input, auto* result) {
        auto path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        result->returnedValue.setBoolean(access(path.c_str(), F_OK) == 0);
    });
    initSystemCall(SystemCall::kFloor, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num.floor();
    });
    initSystemCall(SystemCall::kFlushConsoleOutput, [](const auto& input, auto* /*result*/) {
        input.consoleOutputStream->flush();
    });
    initSystemCall(SystemCall::kHasValue, systemCallHasValue);
    initSystemCall(SystemCall::kHours, systemCallHours);
    initSystemCall(SystemCall::kInputString, systemCallInputString);
    initSystemCall(SystemCall::kListLen, [](const auto& input, auto* result) {
        result->returnedValue.num = dynamic_cast<const ListBase&>(input.getObject(-1)).size();
    });
    initSystemCall(SystemCall::kLog, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num.ln();
    });
    initSystemCall(SystemCall::kLog10, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num.log10();
    });
    initSystemCall(SystemCall::kMilliseconds, [](const auto& input, auto* result) {
        result->returnedValue = input.getValue(-1);  // already in milliseconds!
    });
    initSystemCall(SystemCall::kMinutes, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num * U_MILLIS_PER_MINUTE;
    });
    initSystemCall(SystemCall::kNewLine, [](const auto& /*input*/, auto* result) {
#ifdef _WIN32
        result->returnedObject = boost::make_local_shared<String>("\r\n", 2);
#else
        result->returnedObject = boost::make_local_shared<String>("\n", 1);
#endif
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
    initSystemCall(SystemCall::kObjectEquals, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getObject(-2).equals(input.getObject(-1)) ? 1 : 0;
    });
    initSystemCall(SystemCall::kObjectListAdd, [](const auto& input, auto* result) {
        const auto& objectList = dynamic_cast<const ObjectList&>(input.getObject(-2));
        const auto& object = input.getObjectPtr(-1);
        result->returnedObject = boost::make_local_shared<ObjectList>(objectList, true, objectList.size(), object);
    });
    initSystemCall(SystemCall::kObjectListBuilderNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ObjectListBuilder>();
    });
    initSystemCall(SystemCall::kObjectListBuilderAdd, [](const auto& input, auto* /*result*/) {
        auto& builder = dynamic_cast<ObjectListBuilder&>(input.getObject(-2));
        auto obj = input.getObjectPtr(-1);
        assert(obj->getObjectType() != ObjectType::kObjectListBuilder);
        assert(obj->getObjectType() != ObjectType::kValueListBuilder);
        builder.items.push_back(std::move(obj));
    });
    initSystemCall(SystemCall::kObjectListBuilderEnd, [](const auto& input, auto* result) {
        auto& builder = dynamic_cast<ObjectListBuilder&>(input.getObject(-1));
        result->returnedObject = boost::make_local_shared<ObjectList>(&builder);
    });
    initSystemCall(SystemCall::kObjectListConcat, [](const auto& input, auto* result) {
        const auto& lhs = dynamic_cast<const ObjectList&>(input.getObject(-2));
        const auto& rhs = dynamic_cast<const ObjectList&>(input.getObject(-1));
        ObjectListBuilder builder{ lhs.items.transient() };
        for (const auto& item : rhs.items) {
            builder.items.push_back(item);
        }
        result->returnedObject = boost::make_local_shared<ObjectList>(&builder);
    });
    initSystemCall(SystemCall::kObjectListGet, [](const auto& input, auto* result) {
        const auto& objectList = dynamic_cast<const ObjectList&>(input.getObject(-1));
        const auto& index = input.getValue(-1).getInt64();
        result->returnedObject = objectList.items.at(index);
        assert(result->returnedObject != nullptr);
    });
    initSystemCall(SystemCall::kObjectListSet, [](const auto& input, auto* result) {
        const auto& objectList = dynamic_cast<const ObjectList&>(input.getObject(-2));
        const auto& index = static_cast<size_t>(input.getValue(-1).getInt64());
        const auto& element = input.getObjectPtr(-1);
        result->returnedObject = boost::make_local_shared<ObjectList>(objectList, /* insert */ false, index, element);
    });
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
    initSystemCall(SystemCall::kPathCombine, [](const auto& input, auto* result) {
        const auto& list = dynamic_cast<const ObjectList&>(input.getObject(-1));
        if (list.items.empty()) {
            result->returnedObject = boost::make_local_shared<String>("", 0);
            return;
        }
        auto path = dynamic_cast<const String&>(*list.items.at(0)).toUtf8();
        for (size_t i = 1; i < list.items.size(); i++) {
            auto component = dynamic_cast<const String&>(*list.items.at(i)).toUtf8();
            path = util::pathCombine(path, component);
        }
        result->returnedObject = boost::make_local_shared<String>(path);
    });
    initSystemCall(SystemCall::kPathDirectoryName, [](const auto& input, auto* result) {
        auto path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        result->returnedObject = boost::make_local_shared<String>(util::getDirectoryName(path));
    });
    initSystemCall(SystemCall::kPathExtension, [](const auto& input, auto* result) {
        auto path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        result->returnedObject = boost::make_local_shared<String>(util::getExtension(path));
    });
    initSystemCall(SystemCall::kPathFileName, [](const auto& input, auto* result) {
        auto path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        result->returnedObject = boost::make_local_shared<String>(util::getFileName(path));
    });
    initSystemCall(SystemCall::kPathFileNameWithoutExtension, [](const auto& input, auto* result) {
        auto path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        result->returnedObject = boost::make_local_shared<String>(util::getFileNameWithoutExtension(path));
    });
    initSystemCall(SystemCall::kListDirectories, [](const auto& input, auto* result) {
        auto path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        auto vec = listDirectories(path);
        ObjectListBuilder builder;
        for (const auto& s : vec) {
            builder.items.push_back(boost::make_local_shared<String>(s));
        }
        result->returnedObject = boost::make_local_shared<ObjectList>(&builder);
    });
    initSystemCall(SystemCall::kListFiles, [](const auto& input, auto* result) {
        auto path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        auto vec = listFiles(path);
        ObjectListBuilder builder;
        for (const auto& s : vec) {
            builder.items.push_back(boost::make_local_shared<String>(s));
        }
        result->returnedObject = boost::make_local_shared<ObjectList>(&builder);
    });
    initSystemCall(SystemCall::kListFirst, [](const auto& input, auto* result) {
        systemCallListFirstOrLast(input, result, true);
    });
    initSystemCall(SystemCall::kListLast, [](const auto& input, auto* result) {
        systemCallListFirstOrLast(input, result, false);
    });
    initSystemCall(SystemCall::kPathSeparator, [](const auto& /*input*/, auto* result) {
#ifdef _WIN32
        result->returnedObject = boost::make_local_shared<String>("\\", 1);
#else
        result->returnedObject = boost::make_local_shared<String>("/", 1);
#endif
    });
    initSystemCall(SystemCall::kPow, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num.pow(input.getValue(-1).num);
    });
    initSystemCall(SystemCall::kRound, [](const auto& input, auto* result) {
        result->returnedValue.num = util::round(input.getValue(-1).num);
    });
    initSystemCall(SystemCall::kPrintString, [](const auto& input, auto* /*result*/) {
        *input.consoleOutputStream << dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
    });
    initSystemCall(SystemCall::kReadFileBytes, [](const auto& input, auto* result) {
        auto filePath = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        std::ifstream stream{ filePath };
        stream.seekg(0, std::ios::end);
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }

        std::vector<char> bytes(stream.tellg());
        stream.seekg(0, std::ios::beg);
        stream.read(bytes.data(), bytes.size());
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }

        ValueListBuilder vlb{};
        for (auto& byte : bytes) {
            vlb.items.push_back(Value{ byte });
        }
        result->returnedObject = boost::make_local_shared<ValueList>(&vlb);
    });
    initSystemCall(SystemCall::kReadFileLines, [](const auto& input, auto* result) {
        auto filePath = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        std::ifstream stream{ filePath };
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }
        ObjectListBuilder builder{};
        std::string line;
        while (std::getline(stream, line)) {
            if (stream.fail()) {
                throw Error::fromFileErrno(errno, filePath);
            }
            builder.items.push_back(boost::make_local_shared<String>(line));
        }
        if (stream.fail() && !stream.eof()) {
            throw Error::fromFileErrno(errno, filePath);
        }
        result->returnedObject = boost::make_local_shared<ObjectList>(&builder);
    });
    initSystemCall(SystemCall::kReadFileText, [](const auto& input, auto* result) {
        auto filePath = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        std::ifstream stream{ filePath };
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }
        std::ostringstream ss;
        ss << stream.rdbuf();
        if (stream.fail() && !stream.eof()) {
            throw Error::fromFileErrno(errno, filePath);
        }
        result->returnedObject = boost::make_local_shared<String>(ss.str());
    });
    initSystemCall(SystemCall::kSeconds, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num * U_MILLIS_PER_SECOND;
    });
    initSystemCall(SystemCall::kSin, [](const auto& input, auto* result) {
        result->returnedValue.setDouble(std::sin(input.getValue(-1).getDouble()));
    });
    initSystemCall(SystemCall::kSqr, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num.sqrt();
    });
    initSystemCall(SystemCall::kStringFromCodePoints, [](const auto& input, auto* result) {
        const auto& valueList = dynamic_cast<const ValueList&>(input.getObject(-1));
        auto numCodePoints = valueList.size();
        std::vector<int32_t> codePoints{};
        codePoints.reserve(numCodePoints);
        for (size_t i = 0; i < numCodePoints; ++i) {
            codePoints.push_back(valueList.items.at(i).getInt32());
        }
        result->returnedObject =
            boost::make_local_shared<String>(icu::UnicodeString::fromUTF32(codePoints.data(), codePoints.size()));
    });
    initSystemCall(SystemCall::kStringFromCodeUnits, [](const auto& input, auto* result) {
        const auto& valueList = dynamic_cast<const ValueList&>(input.getObject(-1));
        std::vector<char16_t> uchars(valueList.items.size());
        auto ucharsIter = uchars.begin();
        for (const auto& value : valueList.items) {
            *ucharsIter = static_cast<char16_t>(value.getInt32());
            ucharsIter++;
        }
        icu::UnicodeString ustr{ uchars.data(), static_cast<int32_t>(uchars.size()) };
        result->returnedObject = boost::make_local_shared<String>(std::move(ustr));
    });
    initSystemCall(SystemCall::kStringConcat, [](const auto& input, auto* result) {
        const auto& lhs = dynamic_cast<const String&>(input.getObject(-2));
        const auto& rhs = dynamic_cast<const String&>(input.getObject(-1));
        result->returnedObject = boost::make_local_shared<String>(lhs.toUtf8() + rhs.toUtf8());
    });
    initSystemCall(SystemCall::kStringEquals, [](const auto& input, auto* result) {
        const auto& lhs = dynamic_cast<const String&>(input.getObject(-2));
        const auto& rhs = dynamic_cast<const String&>(input.getObject(-1));
        result->returnedValue.setBoolean(lhs.equals(rhs));
    });
    initSystemCall(SystemCall::kStringLen, systemCallStringLen);
    initSystemCall(SystemCall::kTan, [](const auto& input, auto* result) {
        result->returnedValue.setDouble(std::tan(input.getValue(-1).getDouble()));
    });
    initSystemCall(SystemCall::kTimeSpanToString, [](const auto& input, auto* result) {
        result->returnedObject = timeSpanToString(input.getValue(-1));
    });
    initSystemCall(SystemCall::kTimeZoneFromName, systemCallTimeZoneFromName);
    initSystemCall(SystemCall::kTimeZoneToString, [](const auto& input, auto* result) {
        const auto& timeZone = dynamic_cast<const TimeZone&>(input.getObject(-1));
        icu::UnicodeString name{};
        timeZone.zone->getDisplayName(name);
        result->returnedObject = boost::make_local_shared<String>(std::move(name));
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
    initSystemCall(SystemCall::kTrunc, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num.trunc();
    });
    initSystemCall(SystemCall::kUtcOffset, systemCallUtcOffset);
    initSystemCall(SystemCall::kValue, systemCallValue);
    initSystemCall(SystemCall::kValueListAdd, [](const auto& input, auto* result) {
        const auto& valueList = dynamic_cast<const ValueList&>(input.getObject(-1));
        const auto& value = input.getValue(-1);
        result->returnedObject = boost::make_local_shared<ValueList>(valueList, true, valueList.size(), value);
    });
    initSystemCall(SystemCall::kValueListBuilderNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ValueListBuilder>();
    });
    initSystemCall(SystemCall::kValueListBuilderAdd, [](const auto& input, auto* /*result*/) {
        auto& builder = dynamic_cast<ValueListBuilder&>(input.getObject(-2));
        builder.items.push_back(input.getValue(-1));
    });
    initSystemCall(SystemCall::kValueListBuilderEnd, [](const auto& input, auto* result) {
        auto& builder = dynamic_cast<ValueListBuilder&>(input.getObject(-1));
        result->returnedObject = boost::make_local_shared<ValueList>(&builder);
    });
    initSystemCall(SystemCall::kValueListConcat, [](const auto& input, auto* result) {
        const auto& lhs = dynamic_cast<const ValueList&>(input.getObject(-2));
        const auto& rhs = dynamic_cast<const ValueList&>(input.getObject(-1));
        ValueListBuilder builder{ lhs.items.transient() };
        for (const auto& item : rhs.items) {
            builder.items.push_back(item);
        }
        result->returnedObject = boost::make_local_shared<ValueList>(&builder);
    });
    initSystemCall(SystemCall::kValueListGet, [](const auto& input, auto* result) {
        const auto& valueList = dynamic_cast<const ValueList&>(input.getObject(-1));
        const auto& index = input.getValue(-1).getInt64();
        result->returnedValue = valueList.items.at(index);
    });
    initSystemCall(SystemCall::kValueListSet, [](const auto& input, auto* result) {
        const auto& valueList = dynamic_cast<const ValueList&>(input.getObject(-1));
        const auto& index = static_cast<size_t>(input.getValue(-2).getInt64());
        const auto& value = input.getValue(-1);
        result->returnedObject = boost::make_local_shared<ValueList>(valueList, /* insert */ false, index, value);
    });
    initSystemCall(SystemCall::kValueOptionalNewMissing, [](const auto& /*input*/, auto* result) {
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
    initSystemCall(SystemCall::kWriteFileBytes, [](const auto& input, auto* /*result*/) {
        const auto& filePath = dynamic_cast<const String&>(input.getObject(-2)).toUtf8();
        const auto& bytesValueList = dynamic_cast<const ValueList&>(input.getObject(-1));
        std::vector<char> bytes;
        bytes.reserve(bytesValueList.items.size());
        for (const auto& value : bytesValueList.items) {
            bytes.push_back(static_cast<char>(value.getInt32()));
        }
        std::ofstream stream{ filePath };
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }
        stream.write(bytes.data(), bytes.size());
    });
    initSystemCall(SystemCall::kWriteFileLines, [](const auto& input, auto* /*result*/) {
        const auto& filePath = dynamic_cast<const String&>(input.getObject(-2)).toUtf8();
        const auto& lines = dynamic_cast<const ObjectList&>(input.getObject(-1));
        std::ofstream stream{ filePath };
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }
        for (const auto& line : lines.items) {
            stream << dynamic_cast<const String&>(*line).toUtf8() << kNewLine;
            if (stream.fail()) {
                throw Error::fromFileErrno(errno, filePath);
            }
        }
    });
    initSystemCall(SystemCall::kWriteFileText, [](const auto& input, auto* /*result*/) {
        const auto& filePath = dynamic_cast<const String&>(input.getObject(-2)).toUtf8();
        const auto& text = dynamic_cast<const String&>(input.getObject(-1));
        std::ofstream stream{ filePath };
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }
        stream << text.toUtf8();
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }
    });

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
