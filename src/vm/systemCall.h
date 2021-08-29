#pragma once

#include "../common.h"
#include "vm/Value.h"
#include "vm/Object.h"
#include "vm/constants.h"

namespace vm {

enum class SystemCall {
    kAbs,                           // (x as Number) as Number
    kAcos,                          // (x as Number) as Number
    kAsin,                          // (x as Number) as Number
    kAtan,                          // (x as Number) as Number
    kAtan2,                         // (y as Number, x as Number) as Number
    kAvailableLocales,              // () as List of String
    kAvailableTimeZones,            // () as List of String
    kBooleanAnd,                    // (x as Boolean, y as Boolean) as Boolean
    kBooleanNot,                    // (x as Boolean) as Boolean
    kBooleanOr,                     // (x as Boolean, y as Boolean) as Boolean
    kBooleanToString,               // (x as Boolean) as String
    kCeil,                          // (x as Number) as Number
    kCharacters1,                   // (input as String) as List of String
    kCharacters2,                   // (input as String, locale as String) as List of String
    kChr,                           // (input as Number) as String
    kCodePoints,                    // (input as String) as List of Number
    kCodeUnit1,                     // (input as String) as Number
    kCodeUnit2,                     // (input as String, index as Number) as Number
    kCodeUnits,                     // (input as String) as List of Number
    kConcat1,                       // (input as List of String) as String
    kConcat2,                       // (input as List of String, separator as String) as String
    kCos,                           // (x as Number) as Number
    kCounterIsPastLimit,            // (counter as N, limit as N, step as N) as Boolean
    kCreateDirectory,               // (path as String) as Boolean
    kDateFromParts,                 // (year as Number, month as Number, day as Number) as Date
    kDateTimeFromParts,             // (year as Number, ...) as DateTime
    kDateTimeOffsetFromParts,       // (year as N ... timeZone as TimeZone) as DateTimeOffset
    kDateTimeOffsetToString,        // (dateTimeOffset as DateTimeOffset) as String
    kDateTimeToString,              // (dateTime as DateTime) as String
    kDateToString,                  // (date as Date) as String
    kDays,                          // (count as Number) as TimeSpan
    kDeleteDirectory1,              // (path as String)
    kDeleteDirectory2,              // (path as String, recursive as Boolean)
    kDeleteFile,                    // (filePath as String)
    kErrorCode,                     // () as Number
    kErrorMessage,                  // () as String
    kExp,                           // (x as Number) as Number
    kFileExists,                    // (filePath as String) as Boolean
    kFloor,                         // (x as Number) as Number
    kFlushConsoleOutput,            // ()
    kHasValueO,                     // (input as Optional Object) as Boolean
    kHasValueV,                     // (input as Optional Value) as Boolean
    kHours,                         // (count as Number) as TimeSpan
    kInputString,                   // () as String
    kListDirectories,               // (path as String) as List of String
    kListFiles,                     // (path as String) as List of String
    kListLen,                       // (input as List) as Number
    kLog,                           // (x as Number) as Number
    kLog10,                         // (x as Number) as Number
    kMilliseconds,                  // (count as Number) as TimeSpan
    kMinutes,                       // (count as Number) as TimeSpan
    kNewLine,                       // () as String
    kNumberAdd,                     // (lhs as Number, rhs as Number) as Number
    kNumberDivide,                  // (lhs as Number, rhs as Number) as Number
    kNumberEquals,                  // (lhs as Number, rhs as Number) as Boolean
    kNumberGreaterThan,             // (lhs as Number, rhs as Number) as Boolean
    kNumberGreaterThanEquals,       // (lhs as Number, rhs as Number) as Boolean
    kNumberLessThan,                // (lhs as Number, rhs as Number) as Boolean
    kNumberLessThanEquals,          // (lhs as Number, rhs as Number) as Boolean
    kNumberModulus,                 // (lhs as Number, rhs as Number) as Number
    kNumberMultiply,                // (lhs as Number, rhs as Number) as Number
    kNumberNotEquals,               // (lhs as Number, rhs as Number) as Boolean
    kNumberSubtract,                // (lhs as Number, rhs as Number) as Number
    kNumberToString,                // (input as Number) as String
    kObjectEquals,                  // (lhs as Object, rhs as Object) as Boolean
    kObjectListAdd,                 // (lhs as ObjectList, rhs as Object) as ObjectList
    kObjectListBuilderNew,          // () as ObjectListBuilder
    kObjectListBuilderAdd,          // (builder as ObjectListBuilder, Object as Object)
    kObjectListBuilderEnd,          // (builder as ObjectListBuilder) as ObjectList
    kObjectListConcat,              // (lhs as ObjectList, rhs as ObjectList) as ObjectList
    kObjectListGet,                 // (input as ObjectList, index as Number) as Object
    kObjectListSet,                 // (input as ObjectList, index as Number, value as Object) as ObjectList
    kObjectOptionalNewMissing,      // () as ObjectOptional
    kObjectOptionalNewPresent,      // (input as Object) as ObjectOptional
    kObjectToObjectMapNew,          // () as ObjectToObjectMap
    kObjectToValueMapNew,           // () as ObjectToValueMap
    kPathCombine,                   // (parts as List of String) as String
    kPathDirectoryName,             // (path as String) as String
    kPathExtension,                 // (path as String) as String
    kPathFileName,                  // (path as String) as String
    kPathFileNameWithoutExtension,  // (path as String) as String
    kPathSeparator,                 // () as String
    kPow,                           // (base as Number, exponent as Number) as Number
    kPrintString,                   // (input as String)
    kReadFileBytes,                 // (filePath as String) as List of Number
    kReadFileLines,                 // (filePath as String) as List of String
    kReadFileText,                  // (filePath as String) as String
    kRound,                         // (x as Number) as Number
    kSeconds,                       // (count as Number) as TimeSpan
    kSin,                           // (x as Number) as Number
    kSqr,                           // (x as Number) as Number
    kStringConcat,                  // (lhs as String, rhs as String) as String
    kStringEquals,                  // (lhs as String, rhs as String) as Boolean
    kStringFromCodePoints,          // (codePoints as List of Number) as String
    kStringFromCodeUnits,           // (codeUnits as List of Number) as String
    kStringLen,                     // (input as String) as Number
    kTan,                           // (x as Number) as Number
    kTimeSpanToString,              // (timeSpan as TimeSpan) as String
    kTimeZoneFromName,              // (name as String) as TimeZone
    kTimeZoneToString,              // (timeZone as TimeZone) as String
    kTotalDays,                     // (timeSpan as TimeSpan) as Number
    kTotalHours,                    // (timeSpan as TimeSpan) as Number
    kTotalMilliseconds,             // (timeSpan as TimeSpan) as Number
    kTotalMinutes,                  // (timeSpan as TimeSpan) as Number
    kTotalSeconds,                  // (timeSpan as TimeSpan) as Number
    kTrunc,                         // (x as Number) as Number
    kUtcOffset,                     // (timeZone as TimeZone, dateTime as DateTime) as TimeSpan
    kValueListAdd,                  // (lhs as ValueList, rhs as Value) as ValueList
    kValueListBuilderNew,           // () as ValueListBuilder
    kValueListBuilderAdd,           // (builder as ValueListBuilder, value as Value)
    kValueListBuilderEnd,           // (builder as ValueListBuilder) as ValueList
    kValueListConcat,               // (lhs as ValueList, rhs as ValueList) as ValueList
    kValueListGet,                  // (input as ValueList, index as Number) as Value
    kValueListSet,                  // (input as ValueList, index as Number, value as Value) as ValueList
    kValueO,                        // (input as Optional Object) as Boolean
    kValueOptionalNewMissing,       // () as ValueOptional
    kValueOptionalNewPresent,       // (input as Value) as ValueOptional
    kValueToObjectMapNew,           // () as ValueToObjectMap
    kValueToValueMapNew,            // () as ValueToValueMap
    kValueV,                        // (input as Optional Value) as Boolean
    kWriteFileBytes,                // (filePath as String, bytes as List of Number)
    kWriteFileLines,                // (filePath as String, lines as List of String)
    kWriteFileText,                 // (filePath as String, text as String)
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
