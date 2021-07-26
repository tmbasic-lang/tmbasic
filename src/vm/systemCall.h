#pragma once

#include "../common.h"
#include "vm/Value.h"
#include "vm/Object.h"
#include "vm/constants.h"

namespace vm {

enum class SystemCall {
    kAvailableLocales,          // AvailableLocales() as List of String
    kAvailableTimeZones,        // AvailableTimeZones() as List of String
    kCharacters1,               // Characters(input as String) as List of String
    kCharacters2,               // Characters(input as String, locale as String) as List of String
    kChr,                       // Chr(input as Number) as String
    kCounterIsPastLimit,        // CounterIsPastLimit(counter as Number, limit as Number, step as Number) as Boolean
    kDateFromParts,             // DateFromParts(year as Number, month as Number, day as Number) as Date
    kDateTimeFromParts,         // DateFromParts(year as Number, ...) as DateTime
    kDateTimeOffsetFromParts,   // DateOffsetFromParts(year as Number, ..., timeZone as TimeZone) as DateTimeOffset
    kDateTimeOffsetToString,    // DateTimeOffsetToString(dateTimeOffset as DateTimeOffset) as String
    kDateTimeToString,          // DateTimeToString(dateTime as DateTime) as String
    kDateToString,              // DateToString(date as Date) as String
    kDays,                      // Days(count as Number) as TimeSpan
    kFlushConsoleOutput,        // FlushConsoleOutput()
    kHasValueO,                 // HasValue(input as Optional Object) as Boolean
    kHasValueV,                 // HasValue(input as Optional Value) as Boolean
    kHours,                     // Hours(count as Number) as TimeSpan
    kInputString,               // InputString() as String
    kListLen,                   // ListLen(input as List) as Number
    kMilliseconds,              // Milliseconds(count as Number) as TimeSpan
    kMinutes,                   // Minutes(count as Number) as TimeSpan
    kNumberAdd,                 // NumberAdd(lhs as Number, rhs as Number) as Number
    kNumberDivide,              // NumberDivide(lhs as Number, rhs as Number) as Number
    kNumberEquals,              // NumberEquals(lhs as Number, rhs as Number) as Boolean
    kNumberGreaterThan,         // NumberGreaterThan(lhs as Number, rhs as Number) as Boolean
    kNumberGreaterThanEquals,   // NumberGreaterThanEquals(lhs as Number, rhs as Number) as Boolean
    kNumberLessThan,            // NumberLessThan(lhs as Number, rhs as Number) as Boolean
    kNumberLessThanEquals,      // NumberLessThanEquals(lhs as Number, rhs as Number) as Boolean
    kNumberModulus,             // NumberModulus(lhs as Number, rhs as Number) as Number
    kNumberMultiply,            // NumberMultiply(lhs as Number, rhs as Number) as Number
    kNumberNotEquals,           // NumberNotEquals(lhs as Number, rhs as Number) as Boolean
    kNumberSubtract,            // NumberSubtract(lhs as Number, rhs as Number) as Number
    kNumberToString,            // NumberToString(input as Number) as String
    kObjectListGet,             // ObjectListGet(input as ObjectList, index as Number) as Object
    kObjectListLength,          // ObjectListLength(input as ObjectList) as Number
    kObjectOptionalNewMissing,  // ObjectOptionalNewMissing() as ObjectOptional
    kObjectOptionalNewPresent,  // ObjectOptionalNewPresent(input as Object) as ObjectOptional
    kObjectToObjectMapNew,      // ObjectToObjectMapNew() as ObjectToObjectMap
    kObjectToValueMapNew,       // ObjectToValueMapNew() as ObjectToValueMap
    kPrintString,               // PrintString(input as String)
    kSeconds,                   // Seconds(count as Number) as TimeSpan
    kStringLen,                 // Len(input as String) as Number
    kTimeSpanToString,          // TimeSpanToString(timeSpan as TimeSpan) as String
    kTimeZoneFromName,          // TimeZoneFromName(name as String) as TimeZone
    kTimeZoneToString,          // TimeZoneToString(timeZone as TimeZone) as String
    kTotalDays,                 // TotalDays(timeSpan as TimeSpan) as Number
    kTotalHours,                // TotalHours(timeSpan as TimeSpan) as Number
    kTotalMilliseconds,         // TotalMilliseconds(timeSpan as TimeSpan) as Number
    kTotalMinutes,              // TotalMinutes(timeSpan as TimeSpan) as Number
    kTotalSeconds,              // TotalSeconds(timeSpan as TimeSpan) as Number
    kUtcOffset,                 // UtcOffset(timeZone as TimeZone, dateTime as DateTime) as TimeSpan
    kValueListGet,              // ValueListGet(input as ValueList, index as Number) as Value
    kValueO,                    // ValueO(input as Optional Object) as Boolean
    kValueOptionalNewMissing,   // ValueOptionalNewMissing() as ValueOptional
    kValueOptionalNewPresent,   // ValueOptionalNewPresent(input as Value) as ValueOptional
    kValueToObjectMapNew,       // ValueToObjectMapNew() as ValueToObjectMap
    kValueToValueMapNew,        // ValueToValueMapNew() as ValueToValueMap
    kValueV,                    // ValueV(input as Optional Value) as Boolean
};

class SystemCallInput {
   public:
    const std::array<Value, kValueStackSize>& valueStack;
    const std::array<boost::local_shared_ptr<Object>, kObjectStackSize>& objectStack;
    int valueStackIndex;
    int objectStackIndex;
    std::istream* consoleInputStream;
    std::ostream* consoleOutputStream;
    SystemCallInput(
        const std::array<Value, kValueStackSize>& valueStack,
        const std::array<boost::local_shared_ptr<Object>, kObjectStackSize>& objectStack,
        int valueStackIndex,
        int objectStackIndex,
        std::istream* consoleInputStream,
        std::ostream* consoleOutputStream);
    const Value& getValue(const int vsiOffset) const;
    const Object& getObject(const int osiOffset) const;
    boost::local_shared_ptr<Object> getObjectPtr(const int osiOffset) const;
};

class SystemCallResult {
   public:
    Value returnedValue = {};
    boost::local_shared_ptr<Object> returnedObject = nullptr;
    bool hasError = false;
    std::string errorMessage = "";
    int errorCode = 0;
};

void initSystemCalls();
SystemCallResult systemCall(SystemCall which, const SystemCallInput& input);

}  // namespace vm
