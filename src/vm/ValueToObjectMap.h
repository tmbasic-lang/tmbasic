#pragma once

#include "Object.h"
#include "Value.h"
#include <memory>
#include <unordered_map>

namespace vm {

class ValueToObjectMap : public Object {
   public:
    const std::unordered_map<Value, std::shared_ptr<Object>, ValueHash> pairs;
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}  // namespace vm
