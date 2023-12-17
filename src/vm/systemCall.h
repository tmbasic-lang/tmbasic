#pragma once

#include "../common.h"
#include "vm/Error.h"
#include "vm/Interpreter.h"
#include "vm/Object.h"
#include "vm/Value.h"
#include "vm/String.h"
#include "vm/List.h"
#include "vm/constants.h"

namespace vm {

enum class SystemCall {
    kAbs,                           // (x as Number) as Number
    kAcos,                          // (x as Number) as Number
    kAddControlToForm,              // (form as Form, control as Control)
    kAsin,                          // (x as Number) as Number
    kAtan,                          // (x as Number) as Number
    kAtan2,                         // (y as Number, x as Number) as Number
    kBooleanAnd,                    // (x as Boolean, y as Boolean) as Boolean
    kBooleanNot,                    // (x as Boolean) as Boolean
    kBooleanOr,                     // (x as Boolean, y as Boolean) as Boolean
    kBooleanToString,               // (x as Boolean) as String
    kCeil,                          // (x as Number) as Number
    kCharacters,                    // (input as String) as List of String
    kChr,                           // (input as Number) as String
    kCodePoints,                    // (input as String) as List of Number
    kCodeUnit1,                     // (input as String) as Number
    kCodeUnit2,                     // (input as String, index as Number) as Number
    kCodeUnits,                     // (input as String) as List of Number
    kConcat1,                       // (input as List of String) as String
    kConcat2,                       // (input as List of String, separator as String) as String
    kControlBounds,                 // (control as Control) as Rectangle
    kControlText,                   // (control as Control) as String
    kCos,                           // (x as Number) as Number
    kCounterIsPastLimit,            // (counter as N, limit as N, step as N) as Boolean
    kCreateDirectory,               // (path as String) as Boolean
    kDateFromParts,                 // (year as Number, month as Number, day as Number) as Date
    kDateTimeFromParts,             // (year as Number, ...) as DateTime
    kDateTimeOffsetFromParts,       // (year as N ... timeZone as TimeZone) as DateTimeOffset
    kDateTimeOffsetToDateTime,      // (dateTimeOffset as DateTimeOffset) as DateTime
    kDateTimeOffsetToDate,          // (dateTimeOffset as DateTimeOffset) as Date
    kDateTimeOffsetToString,        // (dateTimeOffset as DateTimeOffset) as String
    kDateTimeToDate,                // (dateTime as DateTime) as Date
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
    kFormTitle,                     // (form as Form) as String
    kHasValue,                      // (input as Optional Any) as Boolean
    kHours,                         // (count as Number) as TimeSpan
    kInputString,                   // () as String
    kIsDigit,                       // (input as String) as Boolean
    kListDirectories,               // (path as String) as List of String
    kListFiles,                     // (path as String) as List of String
    kListFillO,                     // (value as TObject, count as Number) as List of TObject
    kListFillV,                     // (value as TValue, count as Number) as List of TValue
    kListFirst,                     // (list as List of T) as T
    kListLast,                      // (list as List of T) as T
    kListLen,                       // (input as List of T) as Number
    kListMid,                       // (input as List of T, start as Number, count as Number) as List of T
    kListSkip,                      // (input as List of T, count as Number) as List of T
    kListTake,                      // (input as List of T, count as Number) as List of T
    kLog,                           // (x as Number) as Number
    kLog10,                         // (x as Number) as Number
    kMilliseconds,                  // (count as Number) as TimeSpan
    kMinutes,                       // (count as Number) as TimeSpan
    kNewForm,                       // () as Form
    kNewLabel,                      // () as Control
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
    kObjectListBuilderAdd,          // (builder as ObjectListBuilder, Object as Object)
    kObjectListBuilderEnd,          // (builder as ObjectListBuilder) as ObjectList
    kObjectListBuilderNew,          // () as ObjectListBuilder
    kObjectListConcat,              // (lhs as ObjectList, rhs as ObjectList) as ObjectList
    kObjectListGet,                 // (input as ObjectList, index as Number) as Object
    kObjectListSet,                 // (input as ObjectList, index as Number, value as Object) as ObjectList
    kObjectOptionalNewMissing,      // () as ObjectOptional
    kObjectOptionalNewPresent,      // (input as Object) as ObjectOptional
    kObjectToObjectMapNew,          // () as ObjectToObjectMap
    kObjectToValueMapNew,           // () as ObjectToValueMap
    kParseNumber,                   // (input as String) as Number
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
    kRunForm,                       // (form as Form)
    kSeconds,                       // (count as Number) as TimeSpan
    kSetControlBounds1,             // (control as Control, bounds as Rectangle)
    kSetControlBounds2,             // (control as Control, left as Num, top as Num, width as Num, height as Num)
    kSetControlText,                // (control as Control, text as String)
    kSetFormTitle,                  // (form as Form, title as String)
    kSin,                           // (x as Number) as Number
    kSqr,                           // (x as Number) as Number
    kStringConcat,                  // (lhs as String, rhs as String) as String
    kStringEquals,                  // (lhs as String, rhs as String) as Boolean
    kStringFromCodePoints,          // (codePoints as List of Number) as String
    kStringFromCodeUnits,           // (codeUnits as List of Number) as String
    kStringLen,                     // (input as String) as Number
    kStringSplit,                   // (input as String, separator as String) as List of String
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
    kUtcOffsets,                    // (timeZone as TimeZone, dateTime as DateTime) as List of TimeSpan
    kValue,                         // (input as Optional T) as T
    kValueListAdd,                  // (lhs as ValueList, rhs as Value) as ValueList
    kValueListBuilderAdd,           // (builder as ValueListBuilder, value as Value)
    kValueListBuilderEnd,           // (builder as ValueListBuilder) as ValueList
    kValueListBuilderNew,           // () as ValueListBuilder
    kValueListConcat,               // (lhs as ValueList, rhs as ValueList) as ValueList
    kValueListGet,                  // (input as ValueList, index as Number) as Value
    kValueListSet,                  // (input as ValueList, index as Number, value as Value) as ValueList
    kValueOptionalNewMissing,       // () as ValueOptional
    kValueOptionalNewPresent,       // (input as Value) as ValueOptional
    kValueToObjectMapNew,           // () as ValueToObjectMap
    kValueToValueMapNew,            // () as ValueToValueMap
    kWriteFileBytes,                // (filePath as String, bytes as List of Number)
    kWriteFileLines,                // (filePath as String, lines as List of String)
    kWriteFileText,                 // (filePath as String, text as String)
};

class SystemCallInput {
   public:
    Interpreter* interpreter;
    std::array<Value, kValueStackSize>* valueStack;
    std::array<boost::local_shared_ptr<Object>, kObjectStackSize>* objectStack;
    int valueStackIndex;
    int objectStackIndex;
    int numValueArguments;
    int numObjectArguments;
    std::istream* consoleInputStream;
    std::ostream* consoleOutputStream;
    const Value& errorCode;
    const std::string& errorMessage;

    SystemCallInput(
        Interpreter* interpreter,
        std::array<Value, kValueStackSize>* valueStack,
        std::array<boost::local_shared_ptr<Object>, kObjectStackSize>* objectStack,
        int valueStackIndex,
        int objectStackIndex,
        int numValueArguments,
        int numObjectArguments,
        std::istream* consoleInputStream,
        std::ostream* consoleOutputStream,
        const Value& errorCode,
        const std::string& errorMessage);

    inline Value& getValue(const int vsiOffset) const {
        assert(vsiOffset < 0);
        assert(valueStackIndex + vsiOffset >= 0);
        return valueStack->at(valueStackIndex + vsiOffset);
    }

    inline Object& getObject(const int osiOffset) const {
        assert(osiOffset < 0);
        assert(objectStackIndex + osiOffset >= 0);
        const auto& ptr = objectStack->at(objectStackIndex + osiOffset);
        assert(ptr != nullptr);
        return *ptr;
    }

    inline boost::local_shared_ptr<Object> getObjectPtr(const int osiOffset) const {
        return objectStack->at(objectStackIndex + osiOffset);
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

typedef void (*SystemCallFunc)(const SystemCallInput&, SystemCallResult*);

void initSystemCalls();
void initSystemCall(SystemCall which, SystemCallFunc func);
SystemCallResult systemCall(SystemCall which, const SystemCallInput& input);
std::optional<SystemCall> getDualGenericSystemCall(SystemCall call);

}  // namespace vm
