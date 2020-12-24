#pragma once

#include "../../common.h"
#include "shared/vm/ObjectType.h"

namespace vm {

class Object {
   public:
    Object();
    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
    Object(Object&&) = delete;
    Object& operator=(Object&&) = delete;
    virtual ~Object();
    virtual ObjectType getObjectType() const = 0;
    virtual std::size_t getHash() const = 0;
    virtual bool equals(const Object& other) const = 0;
};

struct ObjectPointerCompare {
    bool operator()(const boost::local_shared_ptr<vm::Object>& lhs, const boost::local_shared_ptr<vm::Object>* rhs) {
        return lhs->equals(**rhs);
    }
};

}  // namespace vm

bool operator==(const boost::local_shared_ptr<vm::Object>& lhs, const boost::local_shared_ptr<vm::Object>& rhs);

namespace std {

template <>
struct hash<boost::local_shared_ptr<vm::Object>> {
    std::size_t operator()(const boost::local_shared_ptr<vm::Object>& k) const noexcept { return k->getHash(); }
};

}  // namespace std
