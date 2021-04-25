#include "date.h"
#include "Error.h"
#include "RecordBuilder.h"
#include "util/decimal.h"

namespace vm {

static icu::Calendar* getDefaultCalendar() {
    static std::unique_ptr<icu::Calendar> defaultCalendar{};
    if (defaultCalendar == nullptr) {
        auto icuErrorCode = U_ZERO_ERROR;
        defaultCalendar = std::unique_ptr<icu::Calendar>(icu::Calendar::createInstance(icuErrorCode));
        if (icuErrorCode != U_ZERO_ERROR) {
            throw std::runtime_error("Failed to construct the default calendar.");
        }
    }

    defaultCalendar->clear();
    defaultCalendar->setTimeZone(*icu::TimeZone::getGMT());
    return defaultCalendar.get();
}

Value newDate(int year, int month, int day) {
    auto* cal = getDefaultCalendar();
    cal->set(year, month - 1, day);
    auto icuErrorCode = U_ZERO_ERROR;
    auto udate = cal->getTime(icuErrorCode);
    if (icuErrorCode != U_ZERO_ERROR) {
        throw Error(ErrorCode::kInvalidDateTime, "The date is invalid.");
    }
    return Value(util::doubleToDecimal(udate));
}

Value newDateTime(int year, int month, int day, int hour, int minute, int second, int millisecond) {
    auto* cal = getDefaultCalendar();
    cal->set(year, month - 1, day, hour, minute, second);
    cal->set(UCAL_MILLISECOND, millisecond);
    auto icuErrorCode = U_ZERO_ERROR;
    auto udate = cal->getTime(icuErrorCode);
    if (icuErrorCode != U_ZERO_ERROR) {
        throw Error(ErrorCode::kInvalidDateTime, "The date is invalid.");
    }
    return Value(util::doubleToDecimal(udate));
}

boost::local_shared_ptr<Record> newDateTimeOffset(const Value& dateTime, const Value& timeSpan) {
    auto builder = RecordBuilder(2, 0);
    builder.values.set(0, dateTime);
    builder.values.set(1, timeSpan);
    return boost::make_local_shared<Record>(&builder);
}

}  // namespace vm
