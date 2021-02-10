#include "String.h"

namespace vm {

String::String() : value() {}

String::String(const std::string& utf8) : value(icu::UnicodeString::fromUTF8(utf8)) {}

String::String(icu::UnicodeString utf16) : value(std::move(utf16)) {}

static const char* convertUint8ToInt8(const uint8_t* x) {
    return reinterpret_cast<const char*>(x);  // NOLINT
}

String::String(const uint8_t* source, int length)
    : value(icu::UnicodeString::fromUTF8(icu::StringPiece(convertUint8ToInt8(source), length))) {}

ObjectType String::getObjectType() const {
    return ObjectType::kString;
}

size_t String::getHash() const {
    return value.hashCode();
}

bool String::equals(const Object& other) const {
    if (other.getObjectType() != ObjectType::kString) {
        return false;
    }
    const auto& otherString = dynamic_cast<const String&>(other);
    return (value == otherString.value) != 0;
}

std::string String::toUtf8() const {
    std::string str;
    value.toUTF8String<std::string>(str);
    return str;
}

}  // namespace vm
