#include "Record.h"

using namespace vm;

Kind Record::getKind() const {
    return Kind::kRecord;
}

size_t Record::getHash() const {
    size_t hash = 17;
    auto valuesCount = values.size();
    for (size_t i = 0; i < 5 && i < valuesCount; i++) {
        hash = hash * 23 + std::hash<uint64_t>{}(values[i].bits);
    }
    auto objectsCount = objects.size();
    for (size_t i = 0; i < 5 && i < objectsCount; i++) {
        hash = hash * 23 + objects[i]->getHash();
    }
    return hash;
}

bool Record::equals(const Object& other) const {
    if (other.getKind() != Kind::kRecord) {
        return false;
    }
    auto& otherRecord = (const Record&)other;
    if (values.size() != otherRecord.values.size() || objects.size() != otherRecord.objects.size()) {
        return false;
    }
    auto valuesCount = values.size();
    for (size_t i = 0; i < valuesCount; i++) {
        if (values[i].bits != otherRecord.values[i].bits) {
            return false;
        }
    }
    auto objectsCount = objects.size();
    for (size_t i = 0; i < objectsCount; i++) {
        if (!objects[i]->equals(*otherRecord.objects[i])) {
            return false;
        }
    }
    return true;
}
