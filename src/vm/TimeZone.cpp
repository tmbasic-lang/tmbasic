#include "TimeZone.h"

namespace vm {

TimeZone::TimeZone(boost::local_shared_ptr<icu::SimpleTimeZone> zone) : zone(std::move(zone)) {}

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
    return *zone == *otherTz.zone;
}

}  // namespace vm
