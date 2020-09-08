#include "ObjectList.h"

using namespace vm;

Kind ObjectList::getKind() const {
    return Kind::kObjectList;
}

size_t ObjectList::getHash() const {
    size_t hash = 17;
    auto count = items.size();
    for (size_t i = 0; i < 5 && i < count; i++) {
        hash = hash * 23 + items[i]->getHash();
    }
    return hash;
}

bool ObjectList::equals(const Object& other) const {
    if (other.getKind() != Kind::kObjectList) {
        return false;
    }
    auto& otherList = (const ObjectList&)other;
    if (items.size() != otherList.items.size()) {
        return false;
    }
    auto count = items.size();
    for (size_t i = 0; i < count; i++) {
        if (!items[i]->equals(*otherList.items[i])) {
            return false;
        }
    }
    return true;
}
