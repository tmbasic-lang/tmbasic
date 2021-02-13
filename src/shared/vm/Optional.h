#pragma once

#include "../../common.h"
#include "shared/vm/Object.h"
#include "shared/vm/Value.h"

namespace vm {

class ValueOptional : public Object {
   public:
    const std::optional<Value> item = {};
    ValueOptional();
    ValueOptional(Value value);
    ObjectType getObjectType() const override;
    size_t getHash() const override;
    bool equals(const Object& other) const override;
};

class ObjectOptional : public Object {
   public:
    const std::optional<boost::local_shared_ptr<Object>> item = {};
    ObjectOptional();
    ObjectOptional(boost::local_shared_ptr<Object> object);
    ObjectType getObjectType() const override;
    size_t getHash() const override;
    bool equals(const Object& other) const override;
};

}  // namespace vm
