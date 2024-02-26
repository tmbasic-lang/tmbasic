#pragma once

#include "../common.h"
#include "shared/ObjectType.h"

namespace vm {

class Object {
   public:
    uint32_t referenceCount{ 0 };
    inline Object() = default;
    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
    Object(Object&&) = delete;
    Object& operator=(Object&&) = delete;
    inline virtual ~Object() = default;
    virtual shared::ObjectType getObjectType() const = 0;
    virtual std::size_t getHash() const = 0;
    virtual bool equals(const Object& other) const = 0;
};

inline void intrusive_ptr_add_ref(Object* p) {
    p->referenceCount++;
}

inline void intrusive_ptr_release(Object* p) {
    if (--p->referenceCount == 0) {
        delete p;
    }
}

struct ObjectPointerCompare {
    bool operator()(const boost::intrusive_ptr<vm::Object>& lhs, const boost::intrusive_ptr<vm::Object>* rhs) {
        return lhs->equals(**rhs);
    }
};

struct ObjectReferenceCompare {
    bool operator()(const boost::intrusive_ptr<vm::Object>& lhs, const boost::intrusive_ptr<vm::Object>& rhs) {
        return lhs->equals(*rhs);
    }
};

}  // namespace vm

inline bool operator==(const boost::intrusive_ptr<vm::Object>& lhs, const boost::intrusive_ptr<vm::Object>& rhs) {
    return lhs->equals(*rhs);
}

namespace std {

template <>
struct hash<boost::intrusive_ptr<vm::Object>> {
    std::size_t operator()(const boost::intrusive_ptr<vm::Object>& k) const noexcept { return k->getHash(); }
};

}  // namespace std
