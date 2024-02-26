#pragma once

#include "../common.h"
#include "vm/Object.h"
#include "vm/RecordBuilder.h"
#include "vm/Value.h"

namespace vm {

class Record : public Object {
   public:
    const immer::array<boost::intrusive_ptr<Object>> objects;
    const immer::array<Value> values;
    Record(const Record& source, int valueIndex, Value newValue);
    Record(const Record& source, int objectIndex, const boost::intrusive_ptr<Object>& newObject);
    explicit Record(RecordBuilder* builder);
    shared::ObjectType getObjectType() const override;
    size_t getHash() const override;
    bool equals(const Object& other) const override;
};

}  // namespace vm
