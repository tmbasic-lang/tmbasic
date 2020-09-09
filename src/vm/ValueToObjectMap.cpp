#include "ValueToObjectMap.h"

using namespace vm;

Kind ValueToObjectMap::getKind() const {
    return Kind::kValueToObjectMap;
}

size_t ValueToObjectMap::getHash() const {
    return std::hash<size_t>{}(pairs.size());
}

bool ValueToObjectMap::equals(const Object& other) const {
    if (other.getKind() != Kind::kValueToObjectMap) {
        return false;
    }
    auto& otherMap = (const ValueToObjectMap&)other;
    if (pairs.size() != otherMap.pairs.size()) {
        return false;
    }
    for (auto pair : pairs) {
        auto otherValuePtr = otherMap.pairs.find(pair.first);
        if (otherValuePtr == nullptr) {
            return false;
        }
        if (!pair.second->equals(**otherValuePtr)) {
            return false;
        }
    }
    return true;
}
