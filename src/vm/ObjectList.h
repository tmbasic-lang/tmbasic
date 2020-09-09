#pragma once

#include "common.h"
#include "Object.h"

namespace vm {

class ObjectList : public Object {
   public:
    const immer::vector<boost::local_shared_ptr<Object>> items;
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}  // namespace vm
