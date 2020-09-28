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
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}  // namespace vm
