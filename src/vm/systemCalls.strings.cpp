#include "systemCall.h"
#include "vm/Error.h"
#include "vm/List.h"
#include "vm/String.h"
#include "vm/TimeZone.h"
#include "vm/castObject.h"

namespace vm {

// (x as Boolean) as String
void systemCallBooleanToString(const SystemCallInput& input, SystemCallResult* result) {
    std::string s{ input.getValue(-1).getBoolean() ? "true" : "false" };
    result->returnedObject = boost::make_local_shared<String>(std::move(s));
}

// (input as String) as List of String
void systemCallCharacters(const SystemCallInput& input, SystemCallResult* result) {
    const auto& str = *castString(input.getObject(-1));

    // Fast path if the whole string is ASCII.
    auto fastPath = true;
    for (char ch : str.value) {
        if (ch != 9 && (ch < 32 || ch > 126)) {
            fastPath = false;
            break;
        }
    }

    if (fastPath) {
        ObjectListBuilder objectListBuilder{};
        for (char ch : str.value) {
            objectListBuilder.items.push_back(boost::make_local_shared<String>(std::string{ ch }));
        }
        result->returnedObject = boost::make_local_shared<ObjectList>(&objectListBuilder);
        return;
    }

    // Slow path using libunistring.
    std::vector<char> graphemeBreaks(str.value.size() + 1);
    u8_grapheme_breaks(str.getUnistring(), str.value.size(), graphemeBreaks.data());

    // The libunistring documentation says the first break point is always 1.
    assert(graphemeBreaks[0] == 1);

    ObjectListBuilder objectListBuilder{};
    std::string currentGraphemeCluster{};

    for (size_t i = 0; i < str.value.length(); i++) {
        // Build grapheme clusters based on break points.
        if (graphemeBreaks.at(i) == 1) {
            if (i > 0) {
                objectListBuilder.items.push_back(boost::make_local_shared<String>(currentGraphemeCluster));
            }
            currentGraphemeCluster = {};
        }

        currentGraphemeCluster += str.value.at(i);
    }

    // Add remaining grapheme cluster.
    objectListBuilder.items.push_back(boost::make_local_shared<String>(currentGraphemeCluster));

    result->returnedObject = boost::make_local_shared<ObjectList>(&objectListBuilder);
}

// (input as Number) as String
void systemCallChr(const SystemCallInput& input, SystemCallResult* result) {
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
}

// (input as String) as List of Number
void systemCallCodePoints(const SystemCallInput& input, SystemCallResult* result) {
    const auto& str = *castString(input.getObject(-1));

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
}

// (input as String) as Number
void systemCallCodeUnit1(const SystemCallInput& input, SystemCallResult* result) {
    const auto& str = *castString(input.getObject(-1));
    if (str.value.length() == 0) {
        result->returnedValue.num = 0;
        return;
    }
    auto codeUnit = str.value.at(0);
    result->returnedValue.num = static_cast<int>(codeUnit);
}

// (input as String, index as Number) as Number
void systemCallCodeUnit2(const SystemCallInput& input, SystemCallResult* result) {
    const auto& str = *castString(input.getObject(-1));
    const auto& index = input.getValue(-1).getInt64();
    if (index < 0 || index >= static_cast<int64_t>(str.value.length())) {
        result->returnedValue.num = 0;
        return;
    }
    auto codeUnit = str.value.at(index);
    result->returnedValue.num = static_cast<int>(codeUnit);
}

// (input as String) as List of Number
void systemCallCodeUnits(const SystemCallInput& input, SystemCallResult* result) {
    const auto& str = *castString(input.getObject(-1));
    ValueListBuilder b{};
    size_t numCodeUnits = str.value.length();
    for (size_t i = 0; i < numCodeUnits; i++) {
        b.items.push_back(Value{ static_cast<uint8_t>(str.value.at(i)) });
    }
    result->returnedObject = boost::make_local_shared<ValueList>(&b);
}

// (input as List of String) as String
void systemCallConcat1(const SystemCallInput& input, SystemCallResult* result) {
    const auto& objectList = *castObjectList(input.getObject(-1));

    // Appending to std::string is faster than std::ostringstream here in my testing.
    std::string ss{};
    for (const auto& object : objectList.items) {
        ss += castString(*object).value;
    }

    result->returnedObject = boost::make_local_shared<String>(ss);
}

// (input as List of String, separator as String) as String
void systemCallConcat2(const SystemCallInput& input, SystemCallResult* result) {
    const auto& objectList = *castObjectList(input.getObject(-2));
    const auto& separator = *castString(input.getObject(-1));

    std::ostringstream ss{};
    bool first = true;
    for (const auto& object : objectList.items) {
        if (!first) {
            ss << separator.value;
        }
        ss << castString(*object).value;
        first = false;
    }

    result->returnedObject = boost::make_local_shared<String>(ss.str());
}

// () as Number
void systemCallInputNumber(const SystemCallInput& input, SystemCallResult* result) {
    std::string line;
    std::getline(*input.consoleInputStream, line);
    auto num = util::parseDecimalString(line);
    if (num.isnan()) {
        throw Error(ErrorCode::kInvalidNumberFormat, fmt::format("\"{}\" is not a number.", line));
    }
    result->returnedValue.num = num;
}

// () as String
void systemCallInputString(const SystemCallInput& input, SystemCallResult* result) {
    std::string line;
    std::getline(*input.consoleInputStream, line);
    result->returnedObject = boost::make_local_shared<String>(line);
}

// (input as String) as Boolean
void systemCallIsDigit(const SystemCallInput& input, SystemCallResult* result) {
    const auto& str = *castString(input.getObject(-1));
    result->returnedValue.setBoolean(str.value.length() >= 1 && std::isdigit(str.value.at(0)));
}

// () as String
void systemCallNewLine(const SystemCallInput& /*input*/, SystemCallResult* result) {
#ifdef _WIN32
    result->returnedObject = boost::make_local_shared<String>("\r\n", 2);
#else
    result->returnedObject = boost::make_local_shared<String>("\n", 1);
#endif
}

// (input as String) as Number
void systemCallParseNumber(const SystemCallInput& input, SystemCallResult* result) {
    const auto& str = *castString(input.getObject(-1));
    auto num = util::parseDecimalString(str.value);
    if (num.isnan()) {
        throw Error(ErrorCode::kInvalidNumberFormat, fmt::format("\"{}\" is not a number.", str.value));
    }
    result->returnedValue.num = num;
}

// (input as String)
void systemCallPrintString(const SystemCallInput& input, SystemCallResult* /*result*/) {
    *input.consoleOutputStream << castString(input.getObject(-1))->value;
}

// (codePoints as List of Number) as String
void systemCallStringFromCodePoints(const SystemCallInput& input, SystemCallResult* result) {
    const auto& valueList = *castValueList(input.getObject(-1));
    std::ostringstream ss{};
    std::array<uint8_t, 6> utf8{};

    for (const auto& value : valueList.items) {
        auto codePoint = value.getInt64();

        // Convert the code point to a UTF-8 array.
        auto length = u8_uctomb(utf8.data(), codePoint, utf8.size());
        assert(length <= 6);
        if (length < 0) {
            throw Error(ErrorCode::kInvalidUnicodeCodePoint, fmt::format("Invalid Unicode code point: {}", codePoint));
        }

        // Convert the UTF-8 array to a std::string_view.
        std::string_view sv{ reinterpret_cast<const char*>(utf8.data()), static_cast<size_t>(length) };
        ss << sv;
    }

    result->returnedObject = boost::make_local_shared<String>(ss.str());
}

// (codeUnits as List of Number) as String
void systemCallStringFromCodeUnits(const SystemCallInput& input, SystemCallResult* result) {
    const auto& valueList = *castValueList(input.getObject(-1));

    std::ostringstream ss{};
    int i = 0;
    for (const auto& value : valueList.items) {
        auto codeUnit = value.getInt64();

        if (codeUnit < 0 || codeUnit > 255) {
            throw Error(
                ErrorCode::kInvalidUnicodeCodePoint, fmt::format("Invalid Unicode code unit at string index {}.", i));
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
}

// (lhs as String, rhs as String) as String
void systemCallStringConcat(const SystemCallInput& input, SystemCallResult* result) {
    const auto& lhs = *castString(input.getObject(-2));
    const auto& rhs = *castString(input.getObject(-1));
    result->returnedObject = boost::make_local_shared<String>(lhs.value + rhs.value);
}

// (lhs as String, rhs as String) as Boolean
void systemCallStringEquals(const SystemCallInput& input, SystemCallResult* result) {
    const auto& lhs = *castString(input.getObject(-2));
    const auto& rhs = *castString(input.getObject(-1));
    result->returnedValue.setBoolean(lhs.equals(rhs));
}

// (input as String) as Number
void systemCallStringLen(const SystemCallInput& input, SystemCallResult* result) {
    const auto& str = castString(input.getObject(-1))->value;
    result->returnedValue.num = str.length();
}

// (haystack as String, needle as String, replacement as String) as String
void systemCallStringReplace(const SystemCallInput& input, SystemCallResult* result) {
    const auto& haystack = *castString(input.getObject(-3));
    const auto& needle = *castString(input.getObject(-2));
    const auto& replacement = *castString(input.getObject(-1));

    if (needle.value.length() == 0) {
        throw Error(ErrorCode::kInvalidArgument, "Needle cannot be empty.");
    }

    std::string s{ haystack.value };

    size_t pos = 0;
    while ((pos = s.find(needle.value, pos)) != std::string::npos) {
        s.replace(pos, needle.value.length(), replacement.value);
        pos += replacement.value.length();
    }

    result->returnedObject = boost::make_local_shared<String>(std::move(s));
}

// (input as String, separator as String) as List of String
void systemCallStringSplit(const SystemCallInput& input, SystemCallResult* result) {
    const auto& str = *castString(input.getObject(-2));
    const auto& separator = *castString(input.getObject(-1));

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
}

}  // namespace vm
