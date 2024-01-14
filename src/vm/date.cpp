#include "date.h"
#include "Error.h"
#include "RecordBuilder.h"
#include "shared/decimal.h"
#include "vm/tar.h"

// This is tzdb.tar, the contents of /usr/share/zoneinfo.
extern const char kResourceTzdb[];  // NOLINT(modernize-avoid-c-arrays)
extern const uint kResourceTzdb_len;

namespace vm {

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static bool _isTzdbInitialized = false;

void initializeTzdb() {
    if (!_isTzdbInitialized) {
        untar(kResourceTzdb, static_cast<size_t>(kResourceTzdb_len), absl::AddZoneInfoFile);
        _isTzdbInitialized = true;
    }
}

DateTimeParts::DateTimeParts(
    uint32_t year,
    uint32_t month,
    uint32_t day,
    uint32_t hour,
    uint32_t minute,
    uint32_t second,
    uint32_t millisecond)
    : year(year), month(month), day(day), hour(hour), minute(minute), second(second), millisecond(millisecond) {}

DateTimeOffsetParts::DateTimeOffsetParts(
    uint32_t year,
    uint32_t month,
    uint32_t day,
    uint32_t hour,
    uint32_t minute,
    uint32_t second,
    uint32_t millisecond,
    int64_t utcOffsetMilliseconds)
    : DateTimeParts(year, month, day, hour, minute, second, millisecond),
      utcOffsetMilliseconds(utcOffsetMilliseconds) {}

DateTimeParts::DateTimeParts(const absl::CivilSecond& civilSecond, uint32_t millisecond)
    : year(civilSecond.year()),
      month(civilSecond.month()),
      day(civilSecond.day()),
      hour(civilSecond.hour()),
      minute(civilSecond.minute()),
      second(civilSecond.second()),
      millisecond(millisecond) {}

void DateTimeParts::addYears(int32_t years) {
    absl::CivilYear civilYear{ year };
    civilYear += years;
    year = civilYear.year();
}

void DateTimeParts::addMonths(int32_t months) {
    absl::CivilMonth civilMonth{ year, month };
    civilMonth += months;
    year = civilMonth.year();
    month = civilMonth.month();
}

void DateTimeParts::addDays(int32_t days) {
    absl::CivilDay civilDay{ year, month, day };
    civilDay += days;
    year = civilDay.year();
    month = civilDay.month();
    day = civilDay.day();
}

void DateTimeParts::addHours(int32_t hours) {
    absl::CivilHour civilHour{ year, month, day, hour };
    civilHour += hours;
    year = civilHour.year();
    month = civilHour.month();
    day = civilHour.day();
    hour = civilHour.hour();
}

void DateTimeParts::addMinutes(int32_t minutes) {
    absl::CivilMinute civilMinute{ year, month, day, hour, minute };
    civilMinute += minutes;
    year = civilMinute.year();
    month = civilMinute.month();
    day = civilMinute.day();
    hour = civilMinute.hour();
    minute = civilMinute.minute();
}

void DateTimeParts::addSeconds(int32_t seconds) {
    absl::CivilSecond civilSecond{ year, month, day, hour, minute, second };
    civilSecond += seconds;
    year = civilSecond.year();
    month = civilSecond.month();
    day = civilSecond.day();
    hour = civilSecond.hour();
    minute = civilSecond.minute();
    second = civilSecond.second();
}

void DateTimeParts::addMilliseconds(int32_t milliseconds) {
    // There's no CivilMillisecond so we have to manage this ourselves. Abseil will handle full seconds and we'll
    // handle the extra milliseconds.
    auto totalMilliseconds = static_cast<int64_t>(millisecond) + milliseconds;
    auto overflowSeconds = totalMilliseconds / kMillisecondsPerSecond;
    totalMilliseconds %= kMillisecondsPerSecond;

    // Handle negative milliseconds.
    if (totalMilliseconds < 0) {
        overflowSeconds--;
        totalMilliseconds += kMillisecondsPerSecond;
    }

    addSeconds(static_cast<int32_t>(overflowSeconds));
    millisecond = static_cast<uint32_t>(totalMilliseconds);
}

Value DateTimeOffsetParts::toValue() const {
    // year (0 to 65535)       16 bits
    // month (1 to 12)          4 bits
    // day (1 to 31)            5 bits
    // hour (0 to 23)           5 bits
    // minute (0 to 59)         6 bits
    // second (0 to 59)         6 bits
    // millisecond (0 to 999)  10 bits
    // offset sign (0 or 1)     1 bit
    // offset (0 to 1440)      11 bits
    // --------------------------------
    // Total:                  64 bits

    uint64_t packed = 0;
    packed |= (year & 0xFFFF);

    packed <<= 4;
    packed |= (month & 0xF);

    packed <<= 5;
    packed |= (day & 0x1F);

    packed <<= 5;
    packed |= (hour & 0x1F);

    packed <<= 6;
    packed |= (minute & 0x3F);

    packed <<= 6;
    packed |= (second & 0x3F);

    packed <<= 10;
    packed |= (millisecond & 0x3FF);

    auto utcOffsetMinutes = utcOffsetMilliseconds / kMillisecondsPerMinute;

    packed <<= 1;
    packed |= (utcOffsetMinutes < 0 ? 1 : 0);

    packed <<= 11;
    packed |= (std::abs(utcOffsetMinutes) & 0x7FF);

    return Value{ packed };
}

Value DateTimeParts::toValue() const {
    DateTimeOffsetParts offsetParts{ year, month, day, hour, minute, second, millisecond, 0 };
    return offsetParts.toValue();
}

DateTimeOffsetParts::DateTimeOffsetParts(const Value& value) {
    auto packed = value.getUint64();

    auto offsetMinutes = static_cast<int64_t>(packed & 0x7FF);
    packed >>= 11;

    auto offsetSign = packed & 0x1;
    packed >>= 1;
    if (offsetSign != 0) {
        offsetMinutes = -offsetMinutes;
    }

    millisecond = static_cast<uint32_t>(packed & 0x3FF);
    packed >>= 10;

    second = static_cast<uint32_t>(packed & 0x3F);
    packed >>= 6;

    minute = static_cast<uint32_t>(packed & 0x3F);
    packed >>= 6;

    hour = static_cast<uint32_t>(packed & 0x1F);
    packed >>= 5;

    day = static_cast<uint32_t>(packed & 0x1F);
    packed >>= 5;

    month = static_cast<uint32_t>(packed & 0xF);
    packed >>= 4;

    year = static_cast<uint32_t>(packed & 0xFFFF);

    utcOffsetMilliseconds = offsetMinutes * kMillisecondsPerMinute;
}

DateTimeParts::DateTimeParts(const Value& value) {
    DateTimeOffsetParts parts{ value };
    year = parts.year;
    month = parts.month;
    day = parts.day;
    hour = parts.hour;
    minute = parts.minute;
    second = parts.second;
    millisecond = parts.millisecond;
}

boost::local_shared_ptr<String> dateToString(const Value& date) {
    DateTimeParts parts{ date };
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << parts.year << "-" << std::setw(2) << parts.month << "-" << std::setw(2)
       << parts.day;
    return boost::make_local_shared<String>(ss.str());
}

boost::local_shared_ptr<String> dateTimeToString(const Value& date) {
    DateTimeParts parts{ date };
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << parts.year << "-" << std::setw(2) << parts.month << "-" << std::setw(2)
       << parts.day << " " << std::setw(2) << parts.hour << ":" << std::setw(2) << parts.minute << ":" << std::setw(2)
       << parts.second << "." << std::setw(3) << parts.millisecond;
    return boost::make_local_shared<String>(ss.str());
}

boost::local_shared_ptr<String> dateTimeOffsetToString(const Value& date) {
    DateTimeOffsetParts parts{ date };
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << parts.year << "-" << std::setw(2) << parts.month << "-" << std::setw(2)
       << parts.day << " " << std::setw(2) << parts.hour << ":" << std::setw(2) << parts.minute << ":" << std::setw(2)
       << parts.second << "." << std::setw(3) << parts.millisecond << " ";
    auto offsetHourPart = parts.utcOffsetMilliseconds / kMillisecondsPerHour;
    auto offsetMinutePart = (parts.utcOffsetMilliseconds % kMillisecondsPerHour) / kMillisecondsPerMinute;

    // output offsetHourPart and offsetMinutePart to ss in +HH:MM format
    if (offsetHourPart < 0) {
        ss << "-";
    } else {
        ss << "+";
    }
    ss << std::setw(2) << std::abs(offsetHourPart) << ":" << std::setw(2) << std::abs(offsetMinutePart);

    return boost::make_local_shared<String>(ss.str());
}

boost::local_shared_ptr<String> timeSpanToString(const Value& timeSpan) {
    auto totalMsec = timeSpan.getInt64();
    auto hours = totalMsec / kMillisecondsPerHour;
    auto minutes = (totalMsec - hours * kMillisecondsPerHour) / kMillisecondsPerMinute;
    auto seconds =
        (totalMsec - hours * kMillisecondsPerHour - minutes * kMillisecondsPerMinute) / kMillisecondsPerSecond;
    auto msec =
        totalMsec - hours * kMillisecondsPerHour - minutes * kMillisecondsPerMinute - seconds * kMillisecondsPerSecond;

    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << hours << ":" << std::setw(2) << std::setfill('0') << minutes << ":"
       << std::setw(2) << std::setfill('0') << seconds << "." << std::setw(3) << std::setfill('0') << msec;
    return boost::make_local_shared<String>(ss.str());
}

Value dateTimeToDate(const Value& dateTime) {
    DateTimeParts parts{ dateTime };
    return DateTimeParts{ parts.year, parts.month, parts.day, 0, 0, 0, 0 }.toValue();
}

Value dateTimeOffsetToDateTime(const Value& dateTimeOffset) {
    DateTimeOffsetParts parts{ dateTimeOffset };
    return DateTimeParts{
        parts.year, parts.month, parts.day, parts.hour, parts.minute, parts.second, parts.millisecond
    }
        .toValue();
}

Value dateTimeOffsetToDate(const Value& dateTimeOffset) {
    DateTimeOffsetParts parts{ dateTimeOffset };
    return DateTimeParts{ parts.year, parts.month, parts.day, 0, 0, 0, 0 }.toValue();
}

}  // namespace vm
