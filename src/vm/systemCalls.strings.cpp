#include "systemCall.h"
#include "Error.h"
#include "List.h"
#include "String.h"
#include "TimeZone.h"

namespace vm {

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

void initSystemCallsStrings() {
    initSystemCall(SystemCall::kAvailableLocales, [](const auto& input, auto* result) {
        int32_t count = 0;
        const auto* locales = icu::Locale::getAvailableLocales(count);

        auto objectListBuilder = ObjectListBuilder();
        for (int32_t i = 0; i < count; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            const auto* name = locales[i].getName();
            objectListBuilder.items.push_back(boost::make_local_shared<String>(name, strlen(name)));
        }

        result->returnedObject = boost::make_local_shared<ObjectList>(&objectListBuilder);
    });

    initSystemCall(SystemCall::kBooleanToString, [](const auto& input, auto* result) {
        std::string s{ input.getValue(-1).getBoolean() ? "true" : "false" };
        result->returnedObject = boost::make_local_shared<String>(std::move(s));
    });

    initSystemCall(SystemCall::kCharacters1, [](const auto& input, auto* result) {
        systemCallCharactersCore(input, icu::Locale::getUS(), result);
    });

    initSystemCall(SystemCall::kCharacters2, [](const auto& input, auto* result) {
        const auto& localeName = dynamic_cast<const String&>(input.getObject(-2));
        const auto& locale = getBreakIteratorLocale(localeName.value);
        systemCallCharactersCore(input, locale, result);
    });

    initSystemCall(SystemCall::kChr, [](const auto& input, auto* result) {
        auto value = input.getValue(-1).getInt64();
        auto ch = static_cast<UChar32>(value);
        result->returnedObject =
            boost::make_local_shared<String>(ch > 0 ? icu::UnicodeString(ch) : icu::UnicodeString());
    });

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

    initSystemCall(SystemCall::kInputString, [](const auto& input, auto* result) {
        std::string line;
        std::getline(*input.consoleInputStream, line);
        result->returnedObject = boost::make_local_shared<String>(line);
    });

    initSystemCall(SystemCall::kNewLine, [](const auto& /*input*/, auto* result) {
#ifdef _WIN32
        result->returnedObject = boost::make_local_shared<String>("\r\n", 2);
#else
        result->returnedObject = boost::make_local_shared<String>("\n", 1);
#endif
    });

    initSystemCall(SystemCall::kPrintString, [](const auto& input, auto* /*result*/) {
        *input.consoleOutputStream << dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
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

    initSystemCall(SystemCall::kStringLen, [](const auto& input, auto* result) {
        const auto& str = dynamic_cast<const String&>(input.getObject(-1)).value;
        result->returnedValue.num = str.length();
    });
}

}  // namespace vm
