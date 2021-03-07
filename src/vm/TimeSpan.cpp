#include "TimeSpan.h"

namespace vm {

TimeSpan::TimeSpan() {}

Value TimeSpan::fromDays(const Value& days) {
    return Value(days.num * U_MILLIS_PER_DAY);
}

Value TimeSpan::fromHours(const Value& days) {
    return Value(days.num * U_MILLIS_PER_HOUR);
}

Value TimeSpan::fromMinutes(const Value& days) {
    return Value(days.num * U_MILLIS_PER_MINUTE);
}

Value TimeSpan::fromSeconds(const Value& days) {
    return Value(days.num * U_MILLIS_PER_SECOND);
}

}  // namespace vm
