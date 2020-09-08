#include "ValueToValueMap.h"

using namespace vm;

Kind ValueToValueMap::getKind() const {
    return Kind::kValueToValueMap;
}

size_t ValueToValueMap::getHash() const {
    return std::hash<size_t>{}(pairs.size());
}

bool ValueToValueMap::equals(const Object& other) const {
    if (other.getKind() != Kind::kValueToValueMap) {
        return false;
    }
    auto& otherMap = (const ValueToValueMap&)other;
    if (pairs.size() != otherMap.pairs.size()) {
        return false;
    }
    for (auto pair : pairs) {
        auto otherPair = otherMap.pairs.find(pair.first);
        if (otherPair == otherMap.pairs.end()) {
            return false;
        } else if (pair.second.bits != otherPair->second.bits) {
            return false;
        }
    }
    return true;
}
