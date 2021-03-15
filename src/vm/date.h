#pragma once

#include "../common.h"
#include "Record.h"
#include "Value.h"

namespace vm {

void initDate();
Value newDate(int year, int month, int day);
Value newDateTime(int year, int month, int day, int hour, int minute, int second, int millisecond);
boost::local_shared_ptr<Record> newDateTimeOffset(const Value& dateTime, const Value& timeSpan);

}  // namespace vm
