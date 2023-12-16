#include "systemCall.h"
#include "Error.h"
#include "List.h"
#include "String.h"
#include "TimeZone.h"

namespace vm {

void initSystemCallsStrings() {
    initSystemCall(SystemCall::kBooleanToString, [](const auto& input, auto* result) {
        std::string s{ input.getValue(-1).getBoolean() ? "true" : "false" };
        result->returnedObject = boost::make_local_shared<String>(std::move(s));
    });

    initSystemCall(SystemCall::kCharacters, [](const auto& input, auto* result) {
        const auto& str = dynamic_cast<const String&>(input.getObject(-1));

        std::vector<char> graphemeBreaks(str.value.size() + 1);
        u8_grapheme_breaks(str.getUnistring(), str.value.size(), graphemeBreaks.data());

        // The libunistring documentation says the first break point is always 1.
        assert(graphemeBreaks[0] == 1);

        ObjectListBuilder objectListBuilder{};
        std::ostringstream currentGraphemeCluster{};

        for (size_t i = 0; i < str.value.length(); i++) {
            // Build grapheme clusters based on break points.
            if (graphemeBreaks.at(i) == 1) {
                if (i > 0) {
                    objectListBuilder.items.push_back(boost::make_local_shared<String>(currentGraphemeCluster.str()));
                }
                currentGraphemeCluster = {};
            }

            currentGraphemeCluster << str.value.at(i);
        }

        // Add remaining grapheme cluster.
        objectListBuilder.items.push_back(boost::make_local_shared<String>(currentGraphemeCluster.str()));

        result->returnedObject = boost::make_local_shared<ObjectList>(&objectListBuilder);
    });

    initSystemCall(SystemCall::kChr, [](const auto& input, auto* result) {
        auto value = input.getValue(-1).getInt64();
        auto ch32 = static_cast<ucs4_t>(value);

        // Use u8_uctomb to convert ch32 to a UTF-8 array.
        std::array<uint8_t, 6> utf8{};
        auto length = u8_uctomb(utf8.data(), ch32, utf8.size());
        assert(length <= 6);
        if (length < 0) {
            throw Error(ErrorCode::kInvalidUnicodeCodePoint, fmt::format("Invalid Unicode code point: {}", value));
        }

        // Convert the UTF-8 array to a std::string.
        std::string s{ reinterpret_cast<const char*>(utf8.data()), static_cast<size_t>(length) };

        result->returnedObject = boost::make_local_shared<String>(s);
    });

    initSystemCall(SystemCall::kCodePoints, [](const auto& input, auto* result) {
        const auto& str = dynamic_cast<const String&>(input.getObject(-1));

        // Use u8_mbtouc to convert the UTF-8 string to a vector of code points.
        const auto* utf8 = str.getUnistring();
        auto utf8Length = str.value.length();
        size_t currentIndex = 0;  // Track the index into the string
        ValueListBuilder valueListBuilder{};

        while (currentIndex < utf8Length) {
            ucs4_t ch32 = 0;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            auto length = u8_mbtouc(&ch32, &utf8[currentIndex], utf8Length - currentIndex);
            if (length < 0) {
                throw Error(
                    ErrorCode::kInvalidUnicodeCodePoint,
                    fmt::format("Invalid Unicode code point detected at string index {}.", currentIndex));
            }
            valueListBuilder.items.push_back(Value{ static_cast<int64_t>(ch32) });
            currentIndex += length;
        }

        result->returnedObject = boost::make_local_shared<ValueList>(&valueListBuilder);
    });

    initSystemCall(SystemCall::kCodeUnit1, [](const auto& input, auto* result) {
        const auto& str = dynamic_cast<const String&>(input.getObject(-1));
        if (str.value.length() == 0) {
            result->returnedValue.num = 0;
            return;
        }
        auto codeUnit = str.value.at(0);
        result->returnedValue.num = codeUnit == 0xFFFF ? 0 : codeUnit;
    });

    initSystemCall(SystemCall::kCodeUnit2, [](const auto& input, auto* result) {
        const auto& str = dynamic_cast<const String&>(input.getObject(-1));
        const auto& index = input.getValue(-1).getInt64();
        if (index < 0 || index >= static_cast<int64_t>(str.value.length())) {
            result->returnedValue.num = 0;
            return;
        }
        auto codeUnit = str.value.at(index);
        result->returnedValue.num = codeUnit == 0xFFFF ? 0 : codeUnit;
    });

    initSystemCall(SystemCall::kCodeUnits, [](const auto& input, auto* result) {
        const auto& str = dynamic_cast<const String&>(input.getObject(-1));
        ValueListBuilder b{};
        size_t numCodeUnits = str.value.length();
        for (size_t i = 0; i < numCodeUnits; i++) {
            b.items.push_back(Value{ static_cast<uint8_t>(str.value.at(i)) });
        }
        result->returnedObject = boost::make_local_shared<ValueList>(&b);
    });

    initSystemCall(SystemCall::kConcat1, [](const auto& input, auto* result) {
        const auto& objectList = dynamic_cast<const ObjectList&>(input.getObject(-1));

        std::ostringstream ss{};
        for (const auto& object : objectList.items) {
            ss << dynamic_cast<const String&>(*object).value;
        }

        result->returnedObject = boost::make_local_shared<String>(ss.str());
    });

    initSystemCall(SystemCall::kConcat2, [](const auto& input, auto* result) {
        const auto& objectList = dynamic_cast<const ObjectList&>(input.getObject(-2));
        const auto& separator = dynamic_cast<const String&>(input.getObject(-1));

        std::ostringstream ss{};
        bool first = true;
        for (const auto& object : objectList.items) {
            if (!first) {
                ss << separator.value;
            }
            ss << dynamic_cast<const String&>(*object).value;
            first = false;
        }

        result->returnedObject = boost::make_local_shared<String>(ss.str());
    });

    initSystemCall(SystemCall::kInputString, [](const auto& input, auto* result) {
        std::string line;
        std::getline(*input.consoleInputStream, line);
        result->returnedObject = boost::make_local_shared<String>(line);
    });

    initSystemCall(SystemCall::kIsDigit, [](const auto& input, auto* result) {
        const auto& str = dynamic_cast<const String&>(input.getObject(-1));
        result->returnedValue.setBoolean(str.value.length() >= 1 && std::isdigit(str.value.at(0)));
    });

    initSystemCall(SystemCall::kNewLine, [](const auto& /*input*/, auto* result) {
#ifdef _WIN32
        result->returnedObject = boost::make_local_shared<String>("\r\n", 2);
#else
        result->returnedObject = boost::make_local_shared<String>("\n", 1);
#endif
    });

    initSystemCall(SystemCall::kParseNumber, [](const auto& input, auto* result) {
        const auto& str = dynamic_cast<const String&>(input.getObject(-1));
        auto num = util::parseDecimalString(str.value);
        if (num.isnan()) {
            throw Error(ErrorCode::kInvalidNumberFormat, fmt::format("Invalid number: {}", str.value));
        }
        result->returnedValue.num = num;
    });

    initSystemCall(SystemCall::kPrintString, [](const auto& input, auto* /*result*/) {
        *input.consoleOutputStream << dynamic_cast<const String&>(input.getObject(-1)).value;
    });

    initSystemCall(SystemCall::kStringFromCodePoints, [](const auto& input, auto* result) {
        const auto& valueList = dynamic_cast<const ValueList&>(input.getObject(-1));
        std::ostringstream ss{};
        std::array<uint8_t, 6> utf8{};

        for (const auto& value : valueList.items) {
            auto codePoint = value.getInt64();

            // Convert the code point to a UTF-8 array.
            auto length = u8_uctomb(utf8.data(), codePoint, utf8.size());
            assert(length <= 6);
            if (length < 0) {
                throw Error(
                    ErrorCode::kInvalidUnicodeCodePoint, fmt::format("Invalid Unicode code point: {}", codePoint));
            }

            // Convert the UTF-8 array to a std::string_view.
            std::string_view sv{ reinterpret_cast<const char*>(utf8.data()), static_cast<size_t>(length) };
            ss << sv;
        }

        result->returnedObject = boost::make_local_shared<String>(ss.str());
    });

    initSystemCall(SystemCall::kStringFromCodeUnits, [](const auto& input, auto* result) {
        const auto& valueList = dynamic_cast<const ValueList&>(input.getObject(-1));

        std::ostringstream ss{};
        int i = 0;
        for (const auto& value : valueList.items) {
            auto codeUnit = value.getInt64();

            if (codeUnit < 0 || codeUnit > 255) {
                throw Error(
                    ErrorCode::kInvalidUnicodeCodePoint,
                    fmt::format("Invalid Unicode code unit at string index {}.", i));
            }

            ss << static_cast<char>(codeUnit);
            i++;
        }

        // Validate the UTF-8 string.
        auto str = ss.str();
        const auto* utf8 = reinterpret_cast<const uint8_t*>(str.c_str());
        const auto* error = u8_check(utf8, str.length());
        if (error != nullptr) {
            // error is a pointer to the first invalid unit.
            throw Error(
                ErrorCode::kInvalidUnicodeCodePoint,
                fmt::format("Invalid Unicode code unit detected at string index {}.", error - utf8));
        }

        result->returnedObject = boost::make_local_shared<String>(str);
    });

    initSystemCall(SystemCall::kStringConcat, [](const auto& input, auto* result) {
        const auto& lhs = dynamic_cast<const String&>(input.getObject(-2));
        const auto& rhs = dynamic_cast<const String&>(input.getObject(-1));
        result->returnedObject = boost::make_local_shared<String>(lhs.value + rhs.value);
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

    initSystemCall(SystemCall::kStringSplit, [](const auto& input, auto* result) {
        const auto& str = dynamic_cast<const String&>(input.getObject(-2));
        const auto& separator = dynamic_cast<const String&>(input.getObject(-1));

        if (separator.value.length() == 0) {
            throw Error(ErrorCode::kInvalidArgument, "Separator cannot be empty.");
        }

        ObjectListBuilder builder{};

        std::string token;
        size_t startPos = 0;
        size_t endPos = 0;

        while ((endPos = str.value.find(separator.value, startPos)) != std::string::npos) {
            token = str.value.substr(startPos, endPos - startPos);
            builder.items.push_back(boost::make_local_shared<String>(token));
            startPos = endPos + separator.value.length();
        }

        builder.items.push_back(boost::make_local_shared<String>(str.value.substr(startPos)));
        result->returnedObject = boost::make_local_shared<ObjectList>(&builder);
    });
}

}  // namespace vm
