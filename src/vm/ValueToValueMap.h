#pragma once

#include "Object.h"
#include "Value.h"
#include <unordered_map>

namespace vm {

class ValueToValueMap : public Object {
   public:
    const std::unordered_map<Value, Value, ValueHash> pairs;
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}  // namespace vm
