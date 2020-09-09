#include "ObjectToValueMap.h"

using namespace vm;

Kind ObjectToValueMap::getKind() const {
    return Kind::kObjectToValueMap;
}

size_t ObjectToValueMap::getHash() const {
    return std::hash<size_t>{}(pairs.size());
}

bool ObjectToValueMap::equals(const Object& other) const {
    if (other.getKind() != Kind::kObjectToValueMap) {
        return false;
    }
    auto& otherMap = (const ObjectToValueMap&)other;
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
