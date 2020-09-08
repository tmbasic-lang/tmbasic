#pragma once

#include "Kind.h"
#include <cstddef>

namespace vm {

class Object {
    public:
    virtual Kind getKind() const = 0;
    virtual std::size_t getHash() const = 0;
    virtual bool equals(const Object& other) const = 0;
};

}  // namespace vm
