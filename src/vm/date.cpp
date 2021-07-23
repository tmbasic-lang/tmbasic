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

static icu::Calendar* getCalendar(const Value& value) {
    UDate udate = value.getDouble();
    auto* cal = getDefaultCalendar();
    UErrorCode icuErrorCode = U_ZERO_ERROR;
    cal->setTime(udate, icuErrorCode);
    if (icuErrorCode != U_ZERO_ERROR) {
        throw Error(ErrorCode::kInvalidDateTime, "The date is invalid.");
    }
    return cal;
}

static int32_t getDatePart(const icu::Calendar& cal, UCalendarDateFields field) {
    UErrorCode icuErrorCode = U_ZERO_ERROR;
    auto part = cal.get(field, icuErrorCode);
    if (icuErrorCode != U_ZERO_ERROR) {
        throw Error(ErrorCode::kInvalidDateTime, "ICU error: failed to retrieve a date part.");
    }
    return part;
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

boost::local_shared_ptr<String> dateToString(const Value& date) {
    auto* cal = getCalendar(date);
    auto year = getDatePart(*cal, UCAL_YEAR);
    auto month = getDatePart(*cal, UCAL_MONTH) + 1;
    auto day = getDatePart(*cal, UCAL_DATE);
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << year << "-" << std::setw(2) << month << "-" << std::setw(2) << day;
    return boost::make_local_shared<String>(ss.str());
}

boost::local_shared_ptr<String> dateTimeToString(const Value& date) {
    auto* cal = getCalendar(date);
    auto year = getDatePart(*cal, UCAL_YEAR);
    auto month = getDatePart(*cal, UCAL_MONTH) + 1;
    auto day = getDatePart(*cal, UCAL_DATE);
    auto hour = getDatePart(*cal, UCAL_HOUR_OF_DAY);
    auto minute = getDatePart(*cal, UCAL_MINUTE);
    auto second = getDatePart(*cal, UCAL_SECOND);
    auto millisecond = getDatePart(*cal, UCAL_MILLISECOND);
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << year << "-" << std::setw(2) << month << "-" << std::setw(2) << day << "T"
       << std::setw(2) << hour << ":" << std::setw(2) << minute << ":" << std::setw(2) << second << "." << std::setw(3)
       << millisecond;
    return boost::make_local_shared<String>(ss.str());
}

}  // namespace vm
