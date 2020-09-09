#pragma once

#include "common.h"
#include "Object.h"

namespace vm {

class String : public Object {
   public:
    const std::string value;
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}  // namespace vm
