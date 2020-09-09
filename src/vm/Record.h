#pragma once

#include "common.h"
#include "Object.h"
#include "Value.h"

namespace vm {

class Record : public Object {
    public:
    const immer::array<boost::local_shared_ptr<Object>> objects;
    const immer::array<Value> values;
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}
