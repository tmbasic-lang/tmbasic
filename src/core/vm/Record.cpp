#include "Record.h"

using namespace vm;

Record::Record(RecordBuilder& builder)
    : objects(builder.objects.persistent()), values(builder.values.persistent()) {}

Record::Record(Record& source, int valueIndex, Value newValue)
    : objects(source.objects), values(source.values.set(valueIndex, newValue)) {}

Record::Record(Record& source, int objectIndex, boost::local_shared_ptr<Object>& newObject)
    : objects(source.objects.set(objectIndex, newObject)), values(source.values) {}

ObjectType Record::getObjectType() const {
    return ObjectType::kRecord;
}

size_t Record::getHash() const {
    size_t hash = 17;
    auto valuesCount = values.size();
    for (size_t i = 0; i < 5 && i < valuesCount; i++) {
        hash = hash * 23 + std::hash<int64_t>{}(values[i].num.floor().i64());
    }
    auto objectsCount = objects.size();
    for (size_t i = 0; i < 5 && i < objectsCount; i++) {
        hash = hash * 23 + objects[i]->getHash();
    }
    return hash;
}

bool Record::equals(const Object& other) const {
    if (other.getObjectType() != ObjectType::kRecord) {
        return false;
    }
    auto& otherRecord = (const Record&)other;
    if (values.size() != otherRecord.values.size() || objects.size() != otherRecord.objects.size()) {
        return false;
    }
    auto valuesCount = values.size();
    for (size_t i = 0; i < valuesCount; i++) {
        if (values[i].num != otherRecord.values[i].num) {
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
