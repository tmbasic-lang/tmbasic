#include "systemCall.h"
#include "ErrorCode.h"
#include "List.h"
#include "Optional.h"
#include "String.h"
#include "util/decimal.h"

namespace vm {

typedef void (*SystemCallFunc)(const SystemCallInput&, SystemCallResult*);

static bool systemCallsInitialized = false;
static std::vector<SystemCallFunc> _systemCalls;
static std::unique_ptr<icu::Calendar> _defaultCalendar;  // temp, must be cleared before each use

class Error : public std::runtime_error {
   public:
    ErrorCode code;
    Error(ErrorCode code, const char* message) : code(code), std::runtime_error(message) {}
};

SystemCallInput::SystemCallInput(
    const std::array<Value, kValueStackSize>& valueStack,
    const std::array<boost::local_shared_ptr<Object>, kObjectStackSize>& objectStack,
    int valueStackIndex,
    int objectStackIndex)
    : valueStack(valueStack),
      objectStack(objectStack),
      valueStackIndex(valueStackIndex),
      objectStackIndex(objectStackIndex) {}

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
    auto& str = dynamic_cast<String&>(*input.objectStack.at(input.objectStackIndex));
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
    auto& localeName = dynamic_cast<String&>(*input.objectStack.at(input.objectStackIndex + 1));
    const auto& locale = getBreakIteratorLocale(localeName.value);
    systemCallCharactersCore(input, locale, result);
}

static void systemCallChr(const SystemCallInput& input, SystemCallResult* result) {
    auto value = input.valueStack.at(input.valueStackIndex).getInt64();
    auto ch = static_cast<UChar32>(value);
    result->x = boost::make_local_shared<String>(ch > 0 ? icu::UnicodeString(ch) : icu::UnicodeString());
}

static void systemCallDateFromParts(const SystemCallInput& input, SystemCallResult* result) {
    auto year = input.valueStack.at(input.valueStackIndex).getInt32();
    auto month = input.valueStack.at(input.valueStackIndex + 1).getInt32();
    auto day = input.valueStack.at(input.valueStackIndex + 2).getInt32();
    _defaultCalendar->clear();
    _defaultCalendar->set(year, month - 1, day);
    auto icuErrorCode = U_ZERO_ERROR;
    auto udate = _defaultCalendar->getTime(icuErrorCode);
    if (icuErrorCode != U_ZERO_ERROR) {
        throw Error(ErrorCode::kInvalidDateTime, "The date is invalid.");
    }
    result->a.num = util::doubleToDecimal(udate);
}

static void systemCallDays(const SystemCallInput& input, SystemCallResult* result) {
    result->a.num = input.valueStack.at(input.valueStackIndex).num * U_MILLIS_PER_DAY;
}

static void systemCallHasValueV(const SystemCallInput& input, SystemCallResult* result) {
    auto& opt = dynamic_cast<ValueOptional&>(*input.objectStack.at(input.objectStackIndex));
    result->a.setBoolean(opt.item.has_value());
}

static void systemCallHasValueO(const SystemCallInput& input, SystemCallResult* result) {
    auto& opt = dynamic_cast<ObjectOptional&>(*input.objectStack.at(input.objectStackIndex));
    result->a.setBoolean(opt.item.has_value());
}

static void systemCallHours(const SystemCallInput& input, SystemCallResult* result) {
    result->a.num = input.valueStack.at(input.valueStackIndex).num * U_MILLIS_PER_HOUR;
}

static void systemCallLen(const SystemCallInput& input, SystemCallResult* result) {
    const auto& str = dynamic_cast<String&>(*input.objectStack.at(input.objectStackIndex)).value;
    result->a.num = str.length();
}

static void systemCallMilliseconds(const SystemCallInput& input, SystemCallResult* result) {
    // already in milliseconds!
    result->a = input.valueStack.at(input.valueStackIndex);
}

static void systemCallMinutes(const SystemCallInput& input, SystemCallResult* result) {
    result->a.num = input.valueStack.at(input.valueStackIndex).num * U_MILLIS_PER_MINUTE;
}

static void systemCallSeconds(const SystemCallInput& input, SystemCallResult* result) {
    result->a.num = input.valueStack.at(input.valueStackIndex).num * U_MILLIS_PER_SECOND;
}

static void systemCallTotalDays(const SystemCallInput& input, SystemCallResult* result) {
    result->a.num = input.valueStack.at(input.valueStackIndex).num / U_MILLIS_PER_DAY;
}

static void systemCallTotalHours(const SystemCallInput& input, SystemCallResult* result) {
    result->a.num = input.valueStack.at(input.valueStackIndex).num / U_MILLIS_PER_HOUR;
}

static void systemCallTotalMilliseconds(const SystemCallInput& input, SystemCallResult* result) {
    // already in milliseconds!
    result->a = input.valueStack.at(input.valueStackIndex);
}

static void systemCallTotalMinutes(const SystemCallInput& input, SystemCallResult* result) {
    result->a.num = input.valueStack.at(input.valueStackIndex).num / U_MILLIS_PER_MINUTE;
}

static void systemCallTotalSeconds(const SystemCallInput& input, SystemCallResult* result) {
    result->a.num = input.valueStack.at(input.valueStackIndex).num / U_MILLIS_PER_SECOND;
}

static void systemCallValueV(const SystemCallInput& input, SystemCallResult* result) {
    auto& opt = dynamic_cast<ValueOptional&>(*input.objectStack.at(input.objectStackIndex));
    if (!opt.item.has_value()) {
        throw Error(ErrorCode::kValueNotPresent, "Optional value is not present.");
    }
    result->a = *opt.item;
}

static void systemCallValueO(const SystemCallInput& input, SystemCallResult* result) {
    auto& opt = dynamic_cast<ObjectOptional&>(*input.objectStack.at(input.objectStackIndex));
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
    if (systemCallsInitialized) {
        return;
    }

    initSystemCall(SystemCall::kAvailableLocales, systemCallAvailableLocales);
    initSystemCall(SystemCall::kCharacters1, systemCallCharacters1);
    initSystemCall(SystemCall::kCharacters2, systemCallCharacters2);
    initSystemCall(SystemCall::kChr, systemCallChr);
    initSystemCall(SystemCall::kDateFromParts, systemCallDateFromParts);
    initSystemCall(SystemCall::kDays, systemCallDays);
    initSystemCall(SystemCall::kHasValueO, systemCallHasValueO);
    initSystemCall(SystemCall::kHasValueV, systemCallHasValueV);
    initSystemCall(SystemCall::kHours, systemCallHours);
    initSystemCall(SystemCall::kLen, systemCallLen);
    initSystemCall(SystemCall::kMilliseconds, systemCallMilliseconds);
    initSystemCall(SystemCall::kMinutes, systemCallMinutes);
    initSystemCall(SystemCall::kSeconds, systemCallSeconds);
    initSystemCall(SystemCall::kTotalDays, systemCallTotalDays);
    initSystemCall(SystemCall::kTotalHours, systemCallTotalHours);
    initSystemCall(SystemCall::kTotalMilliseconds, systemCallTotalMilliseconds);
    initSystemCall(SystemCall::kTotalMinutes, systemCallTotalMinutes);
    initSystemCall(SystemCall::kTotalSeconds, systemCallTotalSeconds);
    initSystemCall(SystemCall::kValueO, systemCallValueO);
    initSystemCall(SystemCall::kValueV, systemCallValueV);

    auto icuErrorCode = U_ZERO_ERROR;
    _defaultCalendar = std::unique_ptr<icu::Calendar>(icu::Calendar::createInstance(icuErrorCode));
    if (icuErrorCode != U_ZERO_ERROR) {
        throw std::runtime_error("Failed to construct the default calendar.");
    }

    systemCallsInitialized = true;
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
