#include "String.h"

using namespace vm;

Kind String::getKind() const {
    return Kind::kString;
}

size_t String::getHash() const {
    return std::hash<std::string>{}(value);
}

bool String::equals(const Object& other) const {
    if (other.getKind() != Kind::kString) {
        return false;
    }
    auto& otherString = (const String&)other;
    return value == otherString.value;
}
