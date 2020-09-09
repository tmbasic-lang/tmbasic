#pragma once

#include "common.h"
#include "Object.h"
#include "Value.h"

namespace vm {

class ObjectToObjectMap : public Object {
   public:
    const std::unordered_map<boost::local_shared_ptr<Object>, boost::local_shared_ptr<Object>> pairs;
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}  // namespace vm
