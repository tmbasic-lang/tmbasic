#pragma once

#include "common.h"
#include "Object.h"
#include "Value.h"

namespace vm {

class ValueList : public Object {
   public:
    const immer::vector<Value> items;
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}  // namespace vm
