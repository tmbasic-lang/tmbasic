#pragma once

#include "../common.h"
#include "Record.h"
#include "String.h"
#include "Value.h"

namespace vm {

Value newDate(int year, int month, int day);
Value newDateTime(int year, int month, int day, int hour, int minute, int second, int millisecond);
boost::local_shared_ptr<Record> newDateTimeOffset(const Value& dateTime, const Value& timeSpan);
boost::local_shared_ptr<String> dateToString(const Value& date);
boost::local_shared_ptr<String> dateTimeToString(const Value& dateTime);
boost::local_shared_ptr<String> dateTimeOffsetToString(const Record& dateTimeOffset);
boost::local_shared_ptr<String> timeSpanToString(const Value& timeSpan);
Value dateTimeToDate(const Value& dateTime);

}  // namespace vm
