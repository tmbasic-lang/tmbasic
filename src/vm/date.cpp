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
    UDate udate = value.getInt64();
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
    return Value(static_cast<int64_t>(udate));
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
    return Value(static_cast<int64_t>(udate));
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
    ss << std::setfill('0') << std::setw(4) << year << "-" << std::setw(2) << month << "-" << std::setw(2) << day << " "
       << std::setw(2) << hour << ":" << std::setw(2) << minute << ":" << std::setw(2) << second << "." << std::setw(3)
       << millisecond;
    return boost::make_local_shared<String>(ss.str());
}

boost::local_shared_ptr<String> dateTimeOffsetToString(const Record& date) {
    const auto& dateTime = date.values.at(0);
    auto* cal = getCalendar(dateTime);
    auto year = getDatePart(*cal, UCAL_YEAR);
    auto month = getDatePart(*cal, UCAL_MONTH) + 1;
    auto day = getDatePart(*cal, UCAL_DATE);
    auto hour = getDatePart(*cal, UCAL_HOUR_OF_DAY);
    auto minute = getDatePart(*cal, UCAL_MINUTE);
    auto second = getDatePart(*cal, UCAL_SECOND);
    auto millisecond = getDatePart(*cal, UCAL_MILLISECOND);

    const auto& offsetMsec = date.values.at(1).getInt32();
    auto offsetHourPart = offsetMsec / 3600000;
    auto offsetMinutePart = (offsetMsec % 3600000) / 60000;

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << year << "-" << std::setw(2) << month << "-" << std::setw(2) << day << " "
       << std::setw(2) << hour << ":" << std::setw(2) << minute << ":" << std::setw(2) << second << "." << std::setw(3)
       << millisecond << " ";

    // output offsetHourPart and offsetMinutePart to ss in +HH:MM format
    if (offsetHourPart < 0) {
        ss << "-";
    } else {
        ss << "+";
    }
    ss << std::setw(2) << std::abs(offsetHourPart) << ":" << std::setw(2) << offsetMinutePart;

    return boost::make_local_shared<String>(ss.str());
}

}  // namespace vm
