#pragma once

#include "Object.h"
#include "Value.h"
#include <vector>

namespace vm {

class ValueList : public Object {
   public:
    const std::vector<Value> items;
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}  // namespace vm
