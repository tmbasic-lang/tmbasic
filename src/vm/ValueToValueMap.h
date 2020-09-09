#pragma once

#include "common.h"
#include "Object.h"
#include "Value.h"

namespace vm {

class ValueToValueMap : public Object {
   public:
    const std::unordered_map<Value, Value> pairs;
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}  // namespace vm
