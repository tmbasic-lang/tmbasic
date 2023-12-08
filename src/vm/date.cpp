#include "date.h"
#include "Error.h"
#include "RecordBuilder.h"
#include "util/decimal.h"
#include "vm/tar.h"

// This is tzdb.tar, the contents of /usr/share/zoneinfo.
extern char kResourceTzdb[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResourceTzdb_len;

namespace vm {

static bool is_tzdb_initialized = false;

void initializeTzdb() {
    if (!is_tzdb_initialized) {
        untar(kResourceTzdb, static_cast<size_t>(kResourceTzdb_len), absl::AddZoneInfoFile);
        is_tzdb_initialized = true;
    }
}

Value convertDateTimeOffsetPartsToValue(DateTimeOffsetParts parts) {
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
    packed |= (parts.year & 0xFFFF);

    packed <<= 4;
    packed |= (parts.month & 0xF);

    packed <<= 5;
    packed |= (parts.day & 0x1F);

    packed <<= 5;
    packed |= (parts.hour & 0x1F);

    packed <<= 6;
    packed |= (parts.minute & 0x3F);

    packed <<= 6;
    packed |= (parts.second & 0x3F);

    packed <<= 10;
    packed |= (parts.millisecond & 0x3FF);

    auto utcOffsetMinutes = parts.utcOffsetMilliseconds / MSEC_PER_MINUTE;

    packed <<= 1;
    packed |= (utcOffsetMinutes < 0 ? 1 : 0);

    packed <<= 11;
    packed |= (std::abs(utcOffsetMinutes) & 0x7FF);

    return Value{ packed };
}

Value convertDateTimePartsToValue(DateTimeParts parts) {
    DateTimeOffsetParts offsetParts{
        { parts.year, parts.month, parts.day, parts.hour, parts.minute, parts.second, parts.millisecond }, 0
    };
    return convertDateTimeOffsetPartsToValue(offsetParts);
}

DateTimeOffsetParts convertValueToDateTimeOffsetParts(const Value& value) {
    auto packed = value.getUint64();

    auto offsetMinutes = static_cast<int64_t>(packed & 0x7FF);
    packed >>= 11;

    auto offsetSign = packed & 0x1;
    packed >>= 1;
    if (offsetSign != 0) {
        offsetMinutes = -offsetMinutes;
    }

    auto millisecond = packed & 0x3FF;
    packed >>= 10;

    auto second = packed & 0x3F;
    packed >>= 6;

    auto minute = packed & 0x3F;
    packed >>= 6;

    auto hour = packed & 0x1F;
    packed >>= 5;

    auto day = packed & 0x1F;
    packed >>= 5;

    auto month = packed & 0xF;
    packed >>= 4;

    auto year = packed & 0xFFFF;

    return DateTimeOffsetParts{ { static_cast<uint32_t>(year), static_cast<uint32_t>(month), static_cast<uint32_t>(day),
                                  static_cast<uint32_t>(hour), static_cast<uint32_t>(minute),
                                  static_cast<uint32_t>(second), static_cast<uint32_t>(millisecond) },
                                offsetMinutes * MSEC_PER_MINUTE };
}

DateTimeParts convertValueToDateTimeParts(const Value& value) {
    auto parts = convertValueToDateTimeOffsetParts(value);
    return DateTimeParts{
        parts.year, parts.month, parts.day, parts.hour, parts.minute, parts.second, parts.millisecond
    };
}

boost::local_shared_ptr<String> dateToString(const Value& date) {
    auto parts = convertValueToDateTimeParts(date);
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << parts.year << "-" << std::setw(2) << parts.month << "-" << std::setw(2)
       << parts.day;
    return boost::make_local_shared<String>(ss.str());
}

boost::local_shared_ptr<String> dateTimeToString(const Value& date) {
    auto parts = convertValueToDateTimeParts(date);
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << parts.year << "-" << std::setw(2) << parts.month << "-" << std::setw(2)
       << parts.day << " " << std::setw(2) << parts.hour << ":" << std::setw(2) << parts.minute << ":" << std::setw(2)
       << parts.second << "." << std::setw(3) << parts.millisecond;
    return boost::make_local_shared<String>(ss.str());
}

boost::local_shared_ptr<String> dateTimeOffsetToString(const Value& date) {
    auto parts = convertValueToDateTimeOffsetParts(date);
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << parts.year << "-" << std::setw(2) << parts.month << "-" << std::setw(2)
       << parts.day << " " << std::setw(2) << parts.hour << ":" << std::setw(2) << parts.minute << ":" << std::setw(2)
       << parts.second << "." << std::setw(3) << parts.millisecond << " ";
    auto offsetHourPart = parts.utcOffsetMilliseconds / MSEC_PER_HOUR;
    auto offsetMinutePart = (parts.utcOffsetMilliseconds % MSEC_PER_HOUR) / MSEC_PER_MINUTE;

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
    auto hours = totalMsec / MSEC_PER_HOUR;
    auto minutes = (totalMsec - hours * MSEC_PER_HOUR) / MSEC_PER_MINUTE;
    auto seconds = (totalMsec - hours * MSEC_PER_HOUR - minutes * MSEC_PER_MINUTE) / MSEC_PER_SECOND;
    auto msec = totalMsec - hours * MSEC_PER_HOUR - minutes * MSEC_PER_MINUTE - seconds * MSEC_PER_SECOND;

    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << hours << ":" << std::setw(2) << std::setfill('0') << minutes << ":"
       << std::setw(2) << std::setfill('0') << seconds << "." << std::setw(3) << std::setfill('0') << msec;
    return boost::make_local_shared<String>(ss.str());
}

Value dateTimeToDate(const Value& dateTime) {
    auto parts = convertValueToDateTimeParts(dateTime);
    return convertDateTimePartsToValue(DateTimeParts{ parts.year, parts.month, parts.day, 0, 0, 0, 0 });
}

Value dateTimeOffsetToDateTime(const Value& dateTimeOffset) {
    auto parts = convertValueToDateTimeOffsetParts(dateTimeOffset);
    return convertDateTimePartsToValue(
        DateTimeParts{ parts.year, parts.month, parts.day, parts.hour, parts.minute, parts.second, parts.millisecond });
}

Value dateTimeOffsetToDate(const Value& dateTimeOffset) {
    auto parts = convertValueToDateTimeOffsetParts(dateTimeOffset);
    return convertDateTimePartsToValue(DateTimeParts{ parts.year, parts.month, parts.day, 0, 0, 0, 0 });
}

}  // namespace vm
