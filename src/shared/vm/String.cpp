#include "String.h"

namespace vm {

String::String() : value("") {}

String::String(std::string value) : value(std::move(value)) {}

String::String(const uint8_t* source, int length) : value(reinterpret_cast<const char*>(source), length) {}

ObjectType String::getObjectType() const {
    return ObjectType::kString;
}

size_t String::getHash() const {
    return std::hash<std::string>{}(value);
}

bool String::equals(const Object& other) const {
    if (other.getObjectType() != ObjectType::kString) {
        return false;
    }
    auto& otherString = (const String&)other;
    return value == otherString.value;
}

}  // namespace vm
