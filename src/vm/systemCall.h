#pragma once

#include "../common.h"
#include "vm/Value.h"
#include "vm/Object.h"
#include "vm/constants.h"

namespace vm {

enum class SystemCall {
    kAbs,                       // Abs(x as Number) as Number
    kAcos,                      // Acos(x as Number) as Number
    kAsin,                      // Asin(x as Number) as Number
    kAtan,                      // Atan(x as Number) as Number
    kAtan2,                     // Atan2(y as Number, x as Number) as Number
    kAvailableLocales,          // AvailableLocales() as List of String
    kAvailableTimeZones,        // AvailableTimeZones() as List of String
    kBooleanAnd,                // BooleanAnd(x as Boolean, y as Boolean) as Boolean
    kBooleanNot,                // BooleanNot(x as Boolean) as Boolean
    kBooleanOr,                 // BooleanOr(x as Boolean, y as Boolean) as Boolean
    kBooleanToString,           // BooleanToString(x as Boolean) as String
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
    kCounterIsPastLimit,        // CounterIsPastLimit(counter as N, limit as N, step as N) as Boolean
    kCreateDirectory,           // CreateDirectory(path as String) as Boolean
    kDateFromParts,             // DateFromParts(year as Number, month as Number, day as Number) as Date
    kDateTimeFromParts,         // DateFromParts(year as Number, ...) as DateTime
    kDateTimeOffsetFromParts,   // DateOffsetFromParts(year as N ... timeZone as TimeZone) as DateTimeOffset
    kDateTimeOffsetToString,    // DateTimeOffsetToString(dateTimeOffset as DateTimeOffset) as String
    kDateTimeToString,          // DateTimeToString(dateTime as DateTime) as String
    kDateToString,              // DateToString(date as Date) as String
    kDays,                      // Days(count as Number) as TimeSpan
    kDeleteDirectory1,          // DeleteDirectory(path as String)
    kDeleteDirectory2,          // DeleteDirectory(path as String, recursive as Boolean)
    kDeleteFile,                // DeleteFile(filePath as String)
    kErrorCode,                 // ErrorCode() as Number
    kErrorMessage,              // ErrorMessage() as String
    kExp,                       // Exp(x as Number) as Number
    kFileExists,                // FileExists(filePath as String) as Boolean
    kFloor,                     // Floor(x as Number) as Number
    kFlushConsoleOutput,        // FlushConsoleOutput()
    kHasValueO,                 // HasValue(input as Optional Object) as Boolean
    kHasValueV,                 // HasValue(input as Optional Value) as Boolean
    kHours,                     // Hours(count as Number) as TimeSpan
    kInputString,               // InputString() as String
    kListDirectories,           // ListDirectories(path as String) as List of String
    kListFiles,                 // ListFiles(path as String) as List of String
    kListLen,                   // ListLen(input as List) as Number
    kLog,                       // Log(x as Number) as Number
    kLog10,                     // Log10(x as Number) as Number
    kMilliseconds,              // Milliseconds(count as Number) as TimeSpan
    kMinutes,                   // Minutes(count as Number) as TimeSpan
    kNewLine,                   // NewLine() as String
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
    kObjectEquals,              // ObjectEquals(lhs as Object, rhs as Object) as Boolean
    kObjectListAdd,             // ObjectListAdd(lhs as ObjectList, rhs as Object) as ObjectList
    kObjectListBuilderNew,      // ObjectListBuilderNew() as ObjectListBuilder
    kObjectListBuilderAdd,      // ObjectListBuilderAdd(builder as ObjectListBuilder, Object as Object)
    kObjectListBuilderEnd,      // ObjectListBuilderEnd(builder as ObjectListBuilder) as ObjectList
    kObjectListConcat,          // ObjectListConcat(lhs as ObjectList, rhs as ObjectList) as ObjectList
    kObjectListGet,             // ObjectListGet(input as ObjectList, index as Number) as Object
    kObjectListSet,             // ObjectListSet(input as ObjectList, index as Number, value as Object) as ObjectList
    kObjectOptionalNewMissing,  // ObjectOptionalNewMissing() as ObjectOptional
    kObjectOptionalNewPresent,  // ObjectOptionalNewPresent(input as Object) as ObjectOptional
    kObjectToObjectMapNew,      // ObjectToObjectMapNew() as ObjectToObjectMap
    kObjectToValueMapNew,       // ObjectToValueMapNew() as ObjectToValueMap
    kPathCombine,               // PathCombine(parts as List of String) as String
    kPathDirectoryName,         // PathDirectoryName(path as String) as String
    kPathExtension,             // PathExtension(path as String) as String
    kPathFileName,              // PathFileName(path as String) as String
    kPathFileNameWithoutExtension,  // PathFileNameWithoutExtension(path as String) as String
    kPathSeparator,                 // PathSeparator() as String
    kPow,                           // Pow(base as Number, exponent as Number) as Number
    kPrintString,                   // PrintString(input as String)
    kReadFileBytes,                 // ReadFileBytes(filePath as String) as List of Number
    kReadFileLines,                 // ReadFileLines(filePath as String) as List of String
    kReadFileText,                  // ReadFileText(filePath as String) as String
    kRound,                         // Round(x as Number) as Number
    kSeconds,                       // Seconds(count as Number) as TimeSpan
    kSin,                           // Sin(x as Number) as Number
    kSqr,                           // Sqr(x as Number) as Number
    kStringConcat,                  // StringConcat(lhs as String, rhs as String) as String
    kStringEquals,                  // StringEquals(lhs as String, rhs as String) as Boolean
    kStringFromCodePoints,          // StringFromCodePoints(codePoints as List of Number) as String
    kStringFromCodeUnits,           // StringFromCodeUnits(codeUnits as List of Number) as String
    kStringLen,                     // Len(input as String) as Number
    kTan,                           // Tan(x as Number) as Number
    kTimeSpanToString,              // TimeSpanToString(timeSpan as TimeSpan) as String
    kTimeZoneFromName,              // TimeZoneFromName(name as String) as TimeZone
    kTimeZoneToString,              // TimeZoneToString(timeZone as TimeZone) as String
    kTotalDays,                     // TotalDays(timeSpan as TimeSpan) as Number
    kTotalHours,                    // TotalHours(timeSpan as TimeSpan) as Number
    kTotalMilliseconds,             // TotalMilliseconds(timeSpan as TimeSpan) as Number
    kTotalMinutes,                  // TotalMinutes(timeSpan as TimeSpan) as Number
    kTotalSeconds,                  // TotalSeconds(timeSpan as TimeSpan) as Number
    kTrunc,                         // Trunc(x as Number) as Number
    kUtcOffset,                     // UtcOffset(timeZone as TimeZone, dateTime as DateTime) as TimeSpan
    kValueListAdd,                  // ValueListAdd(lhs as ValueList, rhs as Value) as ValueList
    kValueListBuilderNew,           // ValueListBuilderNew() as ValueListBuilder
    kValueListBuilderAdd,           // ValueListBuilderAdd(builder as ValueListBuilder, value as Value)
    kValueListBuilderEnd,           // ValueListBuilderEnd(builder as ValueListBuilder) as ValueList
    kValueListConcat,               // ValueListConcat(lhs as ValueList, rhs as ValueList) as ValueList
    kValueListGet,                  // ValueListGet(input as ValueList, index as Number) as Value
    kValueListSet,                  // ValueListSet(input as ValueList, index as Number, value as Value) as ValueList
    kValueO,                        // ValueO(input as Optional Object) as Boolean
    kValueOptionalNewMissing,       // ValueOptionalNewMissing() as ValueOptional
    kValueOptionalNewPresent,       // ValueOptionalNewPresent(input as Value) as ValueOptional
    kValueToObjectMapNew,           // ValueToObjectMapNew() as ValueToObjectMap
    kValueToValueMapNew,            // ValueToValueMapNew() as ValueToValueMap
    kValueV,                        // ValueV(input as Optional Value) as Boolean
    kWriteFileBytes,                // WriteFileBytes(filePath as String, bytes as List of Number)
    kWriteFileLines,                // WriteFileLines(filePath as String, lines as List of String)
    kWriteFileText,                 // WriteFileText(filePath as String, text as String)
};

class SystemCallInput {
   public:
    std::array<Value, kValueStackSize>& valueStack;
    std::array<boost::local_shared_ptr<Object>, kObjectStackSize>& objectStack;
    int valueStackIndex;
    int objectStackIndex;
    std::istream* consoleInputStream;
    std::ostream* consoleOutputStream;
    const Value& errorCode;
    const std::string& errorMessage;
    SystemCallInput(
        std::array<Value, kValueStackSize>& valueStack,
        std::array<boost::local_shared_ptr<Object>, kObjectStackSize>& objectStack,
        int valueStackIndex,
        int objectStackIndex,
        std::istream* consoleInputStream,
        std::ostream* consoleOutputStream,
        const Value& errorCode,
        const std::string& errorMessage);
    inline Value& getValue(const int vsiOffset) const {
        assert(vsiOffset < 0);
        assert(valueStackIndex + vsiOffset >= 0);
        return valueStack.at(valueStackIndex + vsiOffset);
    }
    inline Object& getObject(const int osiOffset) const {
        assert(osiOffset < 0);
        assert(objectStackIndex + osiOffset >= 0);
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
