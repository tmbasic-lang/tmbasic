#pragma once

#include "../common.h"
#include "vm/Value.h"
#include "vm/Object.h"
#include "vm/constants.h"

namespace vm {

enum class SystemCall {
    kAbs,                       // Abs(x as Number) as Number
    kAcos,                      // acos(x as Number) as Number
    kAsin,                      // asin(x as Number) as Number
    kAtan,                      // atan(x as Number) as Number
    kAtan2,                     // atan2(y as Number, x as Number) as Number
    kAvailableLocales,          // AvailableLocales() as List of String
    kAvailableTimeZones,        // AvailableTimeZones() as List of String
    kCeil,                      // Ceil(x as Number) as Number
    kCharacters1,               // Characters(input as String) as List of String
    kCharacters2,               // Characters(input as String, locale as String) as List of String
    kChr,                       // Chr(input as Number) as String
    kCodePoints,                // CodePoints(input as String) as List of Number
    kCodeUnit1,                 // CodeUnit1(input as String) as Number
    kCodeUnit2,                 // CodeUnit2(input as String, index as Number) as Number
    kCodeUnits,                 // CodeUnits(input as String) as List of Number
    kConcat1,                   // Concat1(input as List of String) as String
    kConcat2,                   // Concat2(input as List of String, separator as String) as String
    kCos,                       // Cos(x as Number) as Number
    kCounterIsPastLimit,        // CounterIsPastLimit(counter as Number, limit as Number, step as Number) as Boolean
    kDateFromParts,             // DateFromParts(year as Number, month as Number, day as Number) as Date
    kDateTimeFromParts,         // DateFromParts(year as Number, ...) as DateTime
    kDateTimeOffsetFromParts,   // DateOffsetFromParts(year as Number, ..., timeZone as TimeZone) as DateTimeOffset
    kDateTimeOffsetToString,    // DateTimeOffsetToString(dateTimeOffset as DateTimeOffset) as String
    kDateTimeToString,          // DateTimeToString(dateTime as DateTime) as String
    kDateToString,              // DateToString(date as Date) as String
    kDays,                      // Days(count as Number) as TimeSpan
    kDeleteFile,                // DeleteFile(filePath as String) as Boolean
    kErrorCode,                 // ErrorCode() as Number
    kErrorMessage,              // ErrorMessage() as String
    kExp,                       // exp(x as Number) as Number
    kFileExists,                // FileExists(filePath as String) as Boolean
    kFloor,                     // Floor(x as Number) as Number
    kFlushConsoleOutput,        // FlushConsoleOutput()
    kHasValueO,                 // HasValue(input as Optional Object) as Boolean
    kHasValueV,                 // HasValue(input as Optional Value) as Boolean
    kHours,                     // Hours(count as Number) as TimeSpan
    kInputString,               // InputString() as String
    kListLen,                   // ListLen(input as List) as Number
    kLog,                       // Log(x as Number) as Number
    kLog10,                     // Log10(x as Number) as Number
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
    kPow,                       // Pow(base as Number, exponent as Number) as Number
    kPrintString,               // PrintString(input as String)
    kReadFileLines,             // ReadFileLines(filePath as String) as List of String
    kReadFileText,              // ReadFileText(filePath as String) as String
    kRound,                     // Round(x as Number) as Number
    kSeconds,                   // Seconds(count as Number) as TimeSpan
    kSin,                       // Sin(x as Number) as Number
    kSqr,                       // Sqr(x as Number) as Number
    kStringConcat,              // StringConcat(lhs as String, rhs as String) as String
    kStringEquals,              // StringEquals(lhs as String, rhs as String) as Boolean
    kStringFromCodePoints,      // StringFromCodePoints(codePoints as List of Number) as String
    kStringFromCodeUnits,       // StringFromCodeUnits(codeUnits as List of Number) as String
    kStringLen,                 // Len(input as String) as Number
    kTan,                       // Tan(x as Number) as Number
    kTimeSpanToString,          // TimeSpanToString(timeSpan as TimeSpan) as String
    kTimeZoneFromName,          // TimeZoneFromName(name as String) as TimeZone
    kTimeZoneToString,          // TimeZoneToString(timeZone as TimeZone) as String
    kTotalDays,                 // TotalDays(timeSpan as TimeSpan) as Number
    kTotalHours,                // TotalHours(timeSpan as TimeSpan) as Number
    kTotalMilliseconds,         // TotalMilliseconds(timeSpan as TimeSpan) as Number
    kTotalMinutes,              // TotalMinutes(timeSpan as TimeSpan) as Number
    kTotalSeconds,              // TotalSeconds(timeSpan as TimeSpan) as Number
    kTrunc,                     // Trunc(x as Number) as Number
    kUtcOffset,                 // UtcOffset(timeZone as TimeZone, dateTime as DateTime) as TimeSpan
    kValueListGet,              // ValueListGet(input as ValueList, index as Number) as Value
    kValueO,                    // ValueO(input as Optional Object) as Boolean
    kValueOptionalNewMissing,   // ValueOptionalNewMissing() as ValueOptional
    kValueOptionalNewPresent,   // ValueOptionalNewPresent(input as Value) as ValueOptional
    kValueToObjectMapNew,       // ValueToObjectMapNew() as ValueToObjectMap
    kValueToValueMapNew,        // ValueToValueMapNew() as ValueToValueMap
    kValueV,                    // ValueV(input as Optional Value) as Boolean
    kWriteFileLines,            // WriteFileLines(filePath as String, lines as List of String)
    kWriteFileText,             // WriteFileText(filePath as String, text as String)
};

class SystemCallInput {
   public:
    const std::array<Value, kValueStackSize>& valueStack;
    const std::array<boost::local_shared_ptr<Object>, kObjectStackSize>& objectStack;
    int valueStackIndex;
    int objectStackIndex;
    std::istream* consoleInputStream;
    std::ostream* consoleOutputStream;
    const Value& errorCode;
    const std::string& errorMessage;
    SystemCallInput(
        const std::array<Value, kValueStackSize>& valueStack,
        const std::array<boost::local_shared_ptr<Object>, kObjectStackSize>& objectStack,
        int valueStackIndex,
        int objectStackIndex,
        std::istream* consoleInputStream,
        std::ostream* consoleOutputStream,
        const Value& errorCode,
        const std::string& errorMessage);
    inline const Value& getValue(const int vsiOffset) const {
        assert(vsiOffset < 0);
        return valueStack.at(valueStackIndex + vsiOffset);
    }
    inline const Object& getObject(const int osiOffset) const {
        assert(osiOffset < 0);
        const auto& ptr = objectStack.at(objectStackIndex + osiOffset);
        assert(ptr != nullptr);
        return *ptr;
    }
    inline boost::local_shared_ptr<Object> getObjectPtr(const int osiOffset) const {
        return objectStack.at(objectStackIndex + osiOffset);
    }
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
