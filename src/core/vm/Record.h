#pragma once

#include "common.h"
#include "Object.h"
#include "RecordBuilder.h"
#include "Value.h"

namespace vm {

class Record : public Object {
   public:
    const immer::array<boost::local_shared_ptr<Object>> objects;
    const immer::array<Value> values;
    Record(Record& source, int valueIndex, Value newValue);
    Record(Record& source, int objectIndex, boost::local_shared_ptr<Object>& newObject);
    Record(RecordBuilder& builder);
    Kind getKind() const override;
    size_t getHash() const override;
    bool equals(const Object& other) const override;
};

}  // namespace vm
