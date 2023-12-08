#include "String.h"

namespace vm {

String::String() : value() {}

String::String(std::string utf8) : value(std::move(utf8)) {}

String::String(const uint8_t* utf8, int length) : value(reinterpret_cast<const char*>(utf8), length) {}

String::String(const char* utf8, int length) : value(utf8, length) {}

String::String(const char* utf8) : value(utf8) {}

ObjectType String::getObjectType() const {
    return ObjectType::kString;
}

size_t String::getHash() const {
    if (!_hash.has_value()) {
        _hash = std::hash<std::string>{}(value);
    }
    return *_hash;
}

bool String::equals(const Object& other) const {
    if (other.getObjectType() != ObjectType::kString) {
        return false;
    }
    const auto& otherString = dynamic_cast<const String&>(other);
    return (value == otherString.value) != 0;
}

const uint8_t* String::getUnistring() const {
    return reinterpret_cast<const uint8_t*>(value.c_str());
}

}  // namespace vm
