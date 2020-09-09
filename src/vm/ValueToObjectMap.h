#pragma once

#include "common.h"
#include "Object.h"
#include "Value.h"

namespace vm {

class ValueToObjectMap : public Object {
   public:
    const std::unordered_map<Value, boost::local_shared_ptr<Object>> pairs;
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}  // namespace vm
