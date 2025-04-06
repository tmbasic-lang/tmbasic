#include "date.h"
#include "shared/decimal.h"
#include "shared/tar.h"
#include "shared/process.h"
#include "vm/RecordBuilder.h"

// We depend on an internal implementation detail of Abseil in order to statically link our own zoneinfo data.
// However we have evidence that Google uses it in this way, so the risk of breakage should be low.
// See: https://github.com/abseil/abseil-cpp/pull/1626
#include <absl/time/internal/cctz/include/cctz/zone_info_source.h>

using absl::time_internal::cctz::ZoneInfoSource;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static bool _isTzdbInitialized = false;

// An implementation of ZoneInfoSource backed by static in-memory data.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static std::unordered_map<std::string, std::vector<char>> zoneInfoFiles{};

class StaticZoneInfoSource : public ZoneInfoSource {
   public:
    explicit StaticZoneInfoSource(const std::vector<char>& file) : _file(file), _len(file.size()) {}

    std::size_t Read(void* ptr, std::size_t size) override {
        size = std::min(size, _len);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        memcpy(ptr, _file.data() + _offset, size);
        _offset += size;
        _len -= size;
        return size;
    }

    int Skip(std::size_t offset) override {
        offset = std::min(offset, _len);
        _offset += offset;
        _len -= offset;
        return 0;
    }

    std::string Version() const override { return {}; }

   private:
    const std::vector<char>& _file;
    std::size_t _offset = 0;
    std::size_t _len;
};

std::unique_ptr<ZoneInfoSource> customZoneInfoSourceFactory(
    const std::string& name,
    const std::function<std::unique_ptr<ZoneInfoSource>(const std::string& name)>& fallback_factory) {
    assert(_isTzdbInitialized);

    // We accept a dummy name as a test that we're using our static data and not the system data.
    if (name == "tmbasic-dummy-zone") {
        return customZoneInfoSourceFactory("Etc/GMT-4", fallback_factory);
    }

    auto it = zoneInfoFiles.find(name);
    if (it == zoneInfoFiles.end()) {
        return nullptr;
    }
    return std::make_unique<StaticZoneInfoSource>(it->second);
}

static void addStaticZoneInfoFile(const std::string& name, std::vector<char> data) {
    zoneInfoFiles.emplace(name, std::move(data));
}

namespace vm {

void initializeTzdbFromFile() {
    if (!_isTzdbInitialized) {
        // tzdb.dat is a tar file that contains the /usr/share/zoneinfo/ contents.
        auto tarFilePath = shared::getExecutableDirectoryPath() + "/tzdb.dat";

        // Read the contents of the file.
        std::ifstream tarFile(tarFilePath, std::ios::binary);
        std::vector<char> data(std::istreambuf_iterator<char>(tarFile), {});

        absl::time_internal::cctz_extension::zone_info_source_factory = customZoneInfoSourceFactory;
        shared::untar(data.data(), data.size(), addStaticZoneInfoFile);
        _isTzdbInitialized = true;
    }
}

void initializeTzdbFromBuffer(const std::vector<char>& buffer) {
    if (!_isTzdbInitialized) {
        absl::time_internal::cctz_extension::zone_info_source_factory = customZoneInfoSourceFactory;
        shared::untar(buffer.data(), buffer.size(), addStaticZoneInfoFile);
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
    : year(static_cast<uint32_t>(civilSecond.year())),
      month(civilSecond.month()),
      day(civilSecond.day()),
      hour(civilSecond.hour()),
      minute(civilSecond.minute()),
      second(civilSecond.second()),
      millisecond(millisecond) {}

void DateTimeParts::addYears(int32_t years) {
    absl::CivilYear civilYear{ year };
    civilYear += years;
    year = static_cast<uint32_t>(civilYear.year());
}

void DateTimeParts::addMonths(int32_t months) {
    absl::CivilMonth civilMonth{ year, month };
    civilMonth += months;
    year = static_cast<uint32_t>(civilMonth.year());
    month = civilMonth.month();
}

void DateTimeParts::addDays(int32_t days) {
    absl::CivilDay civilDay{ year, month, day };
    civilDay += days;
    year = static_cast<uint32_t>(civilDay.year());
    month = civilDay.month();
    day = civilDay.day();
}

void DateTimeParts::addHours(int32_t hours) {
    absl::CivilHour civilHour{ year, month, day, hour };
    civilHour += hours;
    year = static_cast<uint32_t>(civilHour.year());
    month = civilHour.month();
    day = civilHour.day();
    hour = civilHour.hour();
}

void DateTimeParts::addMinutes(int32_t minutes) {
    absl::CivilMinute civilMinute{ year, month, day, hour, minute };
    civilMinute += minutes;
    year = static_cast<uint32_t>(civilMinute.year());
    month = civilMinute.month();
    day = civilMinute.day();
    hour = civilMinute.hour();
    minute = civilMinute.minute();
}

void DateTimeParts::addSeconds(int32_t seconds) {
    absl::CivilSecond civilSecond{ year, month, day, hour, minute, second };
    civilSecond += seconds;
    year = static_cast<uint32_t>(civilSecond.year());
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
    DateTimeOffsetParts const offsetParts{ year, month, day, hour, minute, second, millisecond, 0 };
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
    DateTimeOffsetParts const parts{ value };
    year = parts.year;
    month = parts.month;
    day = parts.day;
    hour = parts.hour;
    minute = parts.minute;
    second = parts.second;
    millisecond = parts.millisecond;
}

boost::intrusive_ptr<String> dateToString(const Value& date) {
    DateTimeParts const parts{ date };
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << parts.year << "-" << std::setw(2) << parts.month << "-" << std::setw(2)
       << parts.day;
    return boost::make_intrusive_ptr<String>(ss.str());
}

boost::intrusive_ptr<String> dateTimeToString(const Value& date) {
    DateTimeParts const parts{ date };
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << parts.year << "-" << std::setw(2) << parts.month << "-" << std::setw(2)
       << parts.day << " " << std::setw(2) << parts.hour << ":" << std::setw(2) << parts.minute << ":" << std::setw(2)
       << parts.second << "." << std::setw(3) << parts.millisecond;
    return boost::make_intrusive_ptr<String>(ss.str());
}

boost::intrusive_ptr<String> dateTimeOffsetToString(const Value& date) {
    DateTimeOffsetParts const parts{ date };
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

    return boost::make_intrusive_ptr<String>(ss.str());
}

boost::intrusive_ptr<String> timeSpanToString(const Value& timeSpan) {
    std::stringstream ss;

    auto totalMsec = timeSpan.getInt64();

    if (totalMsec < 0) {
        ss << "-";
        totalMsec *= -1;
    }

    auto days = totalMsec / kMillisecondsPerDay;
    totalMsec -= days * kMillisecondsPerDay;
    auto hours = totalMsec / kMillisecondsPerHour;
    totalMsec -= hours * kMillisecondsPerHour;
    auto minutes = totalMsec / kMillisecondsPerMinute;
    totalMsec -= minutes * kMillisecondsPerMinute;
    auto seconds = totalMsec / kMillisecondsPerSecond;
    totalMsec -= seconds * kMillisecondsPerSecond;
    auto msec = totalMsec;

    ss << days << ":" << std::setw(2) << std::setfill('0') << hours << ":" << std::setw(2) << std::setfill('0')
       << minutes << ":" << std::setw(2) << std::setfill('0') << seconds << "." << std::setw(3) << std::setfill('0')
       << msec;

    return boost::make_intrusive_ptr<String>(ss.str());
}

Value dateTimeToDate(const Value& dateTime) {
    DateTimeParts const parts{ dateTime };
    return DateTimeParts{ parts.year, parts.month, parts.day, 0, 0, 0, 0 }.toValue();
}

Value dateTimeOffsetToDateTime(const Value& dateTimeOffset) {
    DateTimeOffsetParts const parts{ dateTimeOffset };
    return DateTimeParts{
        parts.year, parts.month, parts.day, parts.hour, parts.minute, parts.second, parts.millisecond
    }
        .toValue();
}

Value dateTimeOffsetToDate(const Value& dateTimeOffset) {
    DateTimeOffsetParts const parts{ dateTimeOffset };
    return DateTimeParts{ parts.year, parts.month, parts.day, 0, 0, 0, 0 }.toValue();
}

}  // namespace vm
