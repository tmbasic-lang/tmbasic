#include "ValueList.h"

using namespace vm;

Kind ValueList::getKind() const {
    return Kind::kValueList;
}

size_t ValueList::getHash() const {
    size_t hash = 17;
    auto count = items.size();
    for (size_t i = 0; i < 5 && i < count; i++) {
        hash = hash * 23 + std::hash<uint64_t>{}(items[i].bits);
    }
    return hash;
}

bool ValueList::equals(const Object& other) const {
    if (other.getKind() != Kind::kValueList) {
        return false;
    }
    auto& otherList = (const ValueList&)other;
    if (items.size() != otherList.items.size()) {
        return false;
    }
    auto count = items.size();
    for (size_t i = 0; i < count; i++) {
        if (items[i].bits != otherList.items[i].bits) {
            return false;
        }
    }
    return true;
}
