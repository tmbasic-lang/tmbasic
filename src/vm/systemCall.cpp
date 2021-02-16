#include "systemCall.h"
#include "ErrorCode.h"
#include "List.h"
#include "Optional.h"
#include "String.h"

namespace vm {

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

static void systemCallAvailableLocales(SystemCallResult* result) {
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

static void systemCallLen(const SystemCallInput& input, SystemCallResult* result) {
    const auto& str = dynamic_cast<String&>(*input.objectStack.at(input.objectStackIndex)).value;
    result->a.num = str.length();
}

static void systemCallHasValueV(const SystemCallInput& input, SystemCallResult* result) {
    auto& opt = dynamic_cast<ValueOptional&>(*input.objectStack.at(input.objectStackIndex));
    result->a.setBoolean(opt.item.has_value());
}

static void systemCallHasValueO(const SystemCallInput& input, SystemCallResult* result) {
    auto& opt = dynamic_cast<ObjectOptional&>(*input.objectStack.at(input.objectStackIndex));
    result->a.setBoolean(opt.item.has_value());
}

static void systemCallValueV(const SystemCallInput& input, SystemCallResult* result) {
    auto& opt = dynamic_cast<ValueOptional&>(*input.objectStack.at(input.objectStackIndex));
    if (!opt.item.has_value()) {
        throw Error(ErrorCode::kValueNotPresent, "Optional value is not present.");
    }
    result->a = opt.item.value();
}

static void systemCallValueO(const SystemCallInput& input, SystemCallResult* result) {
    auto& opt = dynamic_cast<ObjectOptional&>(*input.objectStack.at(input.objectStackIndex));
    if (!opt.item.has_value()) {
        throw Error(ErrorCode::kValueNotPresent, "Optional value is not present.");
    }
    result->x = opt.item.value();
}

SystemCallResult systemCall(SystemCall which, const SystemCallInput& input) {
    SystemCallResult result;

    try {
        switch (which) {
            case SystemCall::kAvailableLocales:
                systemCallAvailableLocales(&result);
                break;
            case SystemCall::kCharacters1:
                result.popObjects = 1;
                systemCallCharacters1(input, &result);
                break;
            case SystemCall::kCharacters2:
                result.popObjects = 2;
                systemCallCharacters2(input, &result);
                break;
            case SystemCall::kChr:
                result.popValues = 1;
                systemCallChr(input, &result);
                break;
            case SystemCall::kHasValueV:
                result.popObjects = 1;
                systemCallHasValueV(input, &result);
                break;
            case SystemCall::kHasValueO:
                result.popObjects = 1;
                systemCallHasValueO(input, &result);
                break;
            case SystemCall::kLen:
                result.popObjects = 1;
                systemCallLen(input, &result);
                break;
            case SystemCall::kValueV:
                result.popObjects = 1;
                systemCallValueV(input, &result);
                break;
            case SystemCall::kValueO:
                result.popObjects = 1;
                systemCallValueO(input, &result);
                break;
            default:
                assert(false);
        }
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
