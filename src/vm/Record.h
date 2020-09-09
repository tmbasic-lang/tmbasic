#pragma once

#include "common.h"
#include "Object.h"
#include "Value.h"

namespace vm {

class Record : public Object {
    public:
    const std::vector<boost::local_shared_ptr<Object>> objects;
    const std::vector<Value> values;
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}
