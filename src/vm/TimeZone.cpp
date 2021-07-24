#include "TimeZone.h"
#include "util/decimal.h"

namespace vm {

TimeZone::TimeZone(std::unique_ptr<icu::TimeZone> zone) : zone(std::move(zone)) {}

ObjectType TimeZone::getObjectType() const {
    return ObjectType::kTimeZone;
}

size_t TimeZone::getHash() const {
    return std::hash<int32_t>{}(zone->getRawOffset());
}

bool TimeZone::equals(const Object& other) const {
    if (other.getObjectType() != ObjectType::kTimeZone) {
        return false;
    }
    const auto& otherTz = dynamic_cast<const TimeZone&>(other);
    return (*zone == *otherTz.zone) != 0;
}

decimal::Decimal TimeZone::getUtcOffset(const decimal::Decimal& dateTime) const {
    UDate udate = dateTime.floor().i64();
    int32_t rawOffset = 0;
    int32_t dstOffset = 0;
    auto icuError = U_ZERO_ERROR;
    zone->getOffset(udate, 1, rawOffset, dstOffset, icuError);
    return decimal::Decimal(rawOffset + dstOffset);
}

}  // namespace vm
