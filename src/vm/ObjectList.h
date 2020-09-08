#pragma once

#include "Object.h"
#include <memory>
#include <vector>

namespace vm {

class ObjectList : public Object {
   public:
    const std::vector<std::shared_ptr<Object>> items;
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}  // namespace vm
