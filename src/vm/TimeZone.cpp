#include "vm/TimeZone.h"
#include "shared/decimal.h"
#include "vm/castObject.h"
#include "vm/date.h"

using shared::ObjectType;

namespace vm {

TimeZone::TimeZone(std::unique_ptr<absl::TimeZone> zone) : zone(std::move(zone)) {}

ObjectType TimeZone::getObjectType() const {
    return ObjectType::kTimeZone;
}

size_t TimeZone::getHash() const {
    return std::hash<std::string>()(zone->name());
}

bool TimeZone::equals(const Object& other) const {
    if (other.getObjectType() != ObjectType::kTimeZone) {
        return false;
    }
    const auto& otherTz = castTimeZone(other);
    return (*zone == *otherTz.zone) != 0;
}

std::vector<decimal::Decimal> TimeZone::getUtcOffsets(const DateTimeParts& dateTimeParts) const {
    absl::CivilSecond const civilSecond{ dateTimeParts.year, dateTimeParts.month,  dateTimeParts.day,
                                         dateTimeParts.hour, dateTimeParts.minute, dateTimeParts.second };
    auto timeInfo = zone->At(civilSecond);

    // If this is the DST fallback hour, there is both a pre and post time. "pre" is always at least filled.
    std::vector<absl::Time> times{};
    times.push_back(timeInfo.pre);

    // For the DST fallback, the time is ambiguous. It could be either of these, before or after the transition.
    if (timeInfo.kind != absl::TimeZone::TimeInfo::REPEATED) {
        times.push_back(timeInfo.post);
    }

    std::vector<decimal::Decimal> offsets{};

    for (const auto& time : times) {
        auto civilInfo = zone->At(time);

        // Abseil says this access of .offset is "imprudent". Duly noted.
        auto offsetSeconds = civilInfo.offset;

        auto offsetMinutes = offsetSeconds / 60;
        auto offsetDecimal = decimal::Decimal{ offsetMinutes };
        offsets.push_back(offsetDecimal);
    }

    return offsets;
}

}  // namespace vm
