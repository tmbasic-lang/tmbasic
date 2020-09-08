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
        auto otherPair = otherMap.pairs.find(pair.first);
        if (otherPair == otherMap.pairs.end()) {
            return false;
        } else if (!pair.second->equals(*otherPair->second)) {
            return false;
        }
    }
    return true;
}
