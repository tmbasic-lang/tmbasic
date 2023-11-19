#pragma once

#include "../common.h"
#include "vm/Object.h"
#include "vm/Value.h"
#include "vm/date.h"

namespace vm {

class TimeZone : public Object {
   public:
    const std::unique_ptr<absl::TimeZone> zone;
    explicit TimeZone(std::unique_ptr<absl::TimeZone> zone);
    ObjectType getObjectType() const override;
    size_t getHash() const override;
    bool equals(const Object& other) const override;
    std::vector<decimal::Decimal> getUtcOffsets(const DateTimeParts& dateTimeParts) const;
};

}  // namespace vm
