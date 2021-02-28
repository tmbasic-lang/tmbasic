#pragma once

#include "../common.h"
#include "vm/Object.h"
#include "vm/Value.h"

namespace vm {

class ValueOptional : public Object {
   public:
    const std::optional<Value> item = {};
    ValueOptional();
    explicit ValueOptional(Value value);
    ObjectType getObjectType() const override;
    size_t getHash() const override;
    bool equals(const Object& other) const override;
};

class ObjectOptional : public Object {
   public:
    const std::optional<boost::local_shared_ptr<Object>> item = {};
    ObjectOptional();
    explicit ObjectOptional(boost::local_shared_ptr<Object> object);
    ObjectType getObjectType() const override;
    size_t getHash() const override;
    bool equals(const Object& other) const override;
};

}  // namespace vm
