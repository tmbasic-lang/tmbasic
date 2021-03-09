#pragma once

#include "../common.h"
#include "vm/Object.h"

namespace vm {

class TimeZone : public Object {
   public:
    const boost::local_shared_ptr<icu::SimpleTimeZone> zone;
    explicit TimeZone(boost::local_shared_ptr<icu::SimpleTimeZone> zone);
    ObjectType getObjectType() const override;
    size_t getHash() const override;
    bool equals(const Object& other) const override;
};

}  // namespace vm
