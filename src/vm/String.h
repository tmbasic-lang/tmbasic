#pragma once

#include "Object.h"
#include <string>

namespace vm {

class String : public Object {
   public:
    const std::string value;
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}  // namespace vm
