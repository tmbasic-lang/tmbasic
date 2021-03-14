#pragma once

#include "../common.h"
#include "vm/Object.h"
#include "vm/Value.h"

namespace vm {

class TimeZone : public Object {
   public:
    const std::unique_ptr<icu::TimeZone> zone;
    explicit TimeZone(std::unique_ptr<icu::TimeZone> zone);
    ObjectType getObjectType() const override;
    size_t getHash() const override;
    bool equals(const Object& other) const override;
    Value getUtcOffset(const Value& dateTime) const;
};

}  // namespace vm
