#pragma once

#include "../common.h"
#include "vm/ObjectType.h"

namespace vm {

class Object {
   public:
    inline Object() = default;
    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
    Object(Object&&) = delete;
    Object& operator=(Object&&) = delete;
    inline virtual ~Object() = default;
    virtual ObjectType getObjectType() const = 0;
    virtual std::size_t getHash() const = 0;
    virtual bool equals(const Object& other) const = 0;
};

struct ObjectPointerCompare {
    bool operator()(const boost::local_shared_ptr<vm::Object>& lhs, const boost::local_shared_ptr<vm::Object>* rhs) {
        return lhs->equals(**rhs);
    }
};

struct ObjectReferenceCompare {
    bool operator()(const boost::local_shared_ptr<vm::Object>& lhs, const boost::local_shared_ptr<vm::Object>& rhs) {
        return lhs->equals(*rhs);
    }
};

}  // namespace vm

inline bool operator==(const boost::local_shared_ptr<vm::Object>& lhs, const boost::local_shared_ptr<vm::Object>& rhs) {
    return lhs->equals(*rhs);
}

namespace std {

template <>
struct hash<boost::local_shared_ptr<vm::Object>> {
    std::size_t operator()(const boost::local_shared_ptr<vm::Object>& k) const noexcept { return k->getHash(); }
};

}  // namespace std
