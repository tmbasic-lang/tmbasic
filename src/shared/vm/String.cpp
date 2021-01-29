#include "String.h"

namespace vm {

static std::u32string toUtf32(const std::string& str) {
    return std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(str);
}

String::String() : value({}) {}

String::String(const std::string& value) : value(toUtf32(value)) {}

String::String(std::u32string value) : value(std::move(value)) {}

static const char* convertUint8ToInt8(const uint8_t* x) {
    return reinterpret_cast<const char*>(x);  // NOLINT
}

String::String(const uint8_t* source, int length) : value(toUtf32(std::string(convertUint8ToInt8(source), length))) {}

ObjectType String::getObjectType() const {
    return ObjectType::kString;
}

size_t String::getHash() const {
    return std::hash<std::u32string>{}(value);
}

bool String::equals(const Object& other) const {
    if (other.getObjectType() != ObjectType::kString) {
        return false;
    }
    const auto& otherString = dynamic_cast<const String&>(other);
    return value == otherString.value;
}

std::string String::toUtf8() const {
    return std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.to_bytes(value);
}

}  // namespace vm
