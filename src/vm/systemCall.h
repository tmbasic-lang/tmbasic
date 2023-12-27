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

// This enum is big enough that it requires NAMEOF_ENUM_RANGE_MAX to be increased.
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
    kDateTimeOffsetDay,             // (dateTimeOffset as DateTimeOffset) as Number
    kDateTimeOffsetFromParts,       // (year as N ... timeZone as TimeZone) as DateTimeOffset
    kDateTimeOffsetHour,            // (dateTimeOffset as DateTimeOffset) as Number
    kDateTimeOffsetMillisecond,     // (dateTimeOffset as DateTimeOffset) as Number
    kDateTimeOffsetMinute,          // (dateTimeOffset as DateTimeOffset) as Number
    kDateTimeOffsetMonth,           // (dateTimeOffset as DateTimeOffset) as Number
    kDateTimeOffsetSecond,          // (dateTimeOffset as DateTimeOffset) as Number
    kDateTimeOffsetToDate,          // (dateTimeOffset as DateTimeOffset) as Date
    kDateTimeOffsetToDateTime,      // (dateTimeOffset as DateTimeOffset) as DateTime
    kDateTimeOffsetToString,        // (dateTimeOffset as DateTimeOffset) as String
    kDateTimeOffsetYear,            // (dateTimeOffset as DateTimeOffset) as Number
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
    kInputNumber,                   // () as Number
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
    kListRemoveAt1,                 // (collection as List of T, index as Number) as List of T
    kListRemoveAt2,                 // (collection as List of T, indices as List of Number) as List of T
    kListSkip,                      // (input as List of T, count as Number) as List of T
    kListTake,                      // (input as List of T, count as Number) as List of T
    kLog,                           // (x as Number) as Number
    kLog10,                         // (x as Number) as Number
    kMapContainsKey,                // (input as Map from K to V, key as K) as Boolean
    kMapFind,                       // (input as Map from K to V, key as K) as Optional V/
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
    kObjectListRemove,              // (input as ObjectList, item as Object) as ObjectList
    kObjectListSet,                 // (input as ObjectList, index as Number, value as Object) as ObjectList
    kObjectOptionalNewMissing,      // () as ObjectOptional
    kObjectOptionalNewPresent,      // (input as Object) as ObjectOptional
    kObjectSetAdd,                  // (lhs as ObjectSet, rhs as Object) as ObjectSet
    kObjectSetBuilderAdd,           // (builder as ObjectSetBuilder, Object as Object)
    kObjectSetBuilderEnd,           // (builder as ObjectSetBuilder) as ObjectSet
    kObjectSetBuilderNew,           // () as ObjectSetBuilder
    kObjectSetNew,                  // () as ObjectSet
    kObjectSetRemove,               // (lhs as ObjectSet, rhs as Object) as ObjectSet
    kObjectSetUnion,                // (lhs as Set of T, rhs as Set of T) as Set of T
    kObjectToObjectMapNew,          // () as ObjectToObjectMap
    kObjectToObjectMapSet,          // (input as ObjectToObjectMap, key as Object, value as Object) as ObjectToObjectMap
    kObjectToObjectMapBuilderAdd,   // (builder as ObjectToObjectMapBuilder, key as Object, value as Object)
    kObjectToObjectMapBuilderEnd,   // (builder as ObjectToObjectMapBuilder) as ObjectToObjectMap
    kObjectToObjectMapBuilderNew,   // () as ObjectToObjectMapBuilder
    kObjectToValueMapNew,           // () as ObjectToValueMap
    kObjectToValueMapSet,           // (input as ObjectToValueMap, key as Object, value as Value) as ObjectToValueMap
    kObjectToValueMapBuilderAdd,    // (builder as ObjectToValueMapBuilder, key as Object, value as Value)
    kObjectToValueMapBuilderEnd,    // (builder as ObjectToValueMapBuilder) as ObjectToValueMap
    kObjectToValueMapBuilderNew,    // () as ObjectToValueMapBuilder
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
    kSetContains,                   // (input as Set of T, value as T) as Boolean
    kSetControlBounds1,             // (control as Control, bounds as Rectangle)
    kSetControlBounds2,             // (control as Control, left as Num, top as Num, width as Num, height as Num)
    kSetControlText,                // (control as Control, text as String)
    kSetFormTitle,                  // (form as Form, title as String)
    kSetLen,                        // (input as Set of T) as Number
    kSetValues,                     // (input as Set of T) as List of T
    kSin,                           // (x as Number) as Number
    kSqr,                           // (x as Number) as Number
    kStringConcat,                  // (lhs as String, rhs as String) as String
    kStringEquals,                  // (lhs as String, rhs as String) as Boolean
    kStringFromCodePoints,          // (codePoints as List of Number) as String
    kStringFromCodeUnits,           // (codeUnits as List of Number) as String
    kStringLen,                     // (input as String) as Number
    kStringReplace,                 // (haystack as String, needle as String, replacement as String) as String
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
    kValueListRemove,               // (input as ValueList, item as Value) as ValueList
    kValueListSet,                  // (input as ValueList, index as Number, value as Value) as ValueList
    kValueOptionalNewMissing,       // () as ValueOptional
    kValueOptionalNewPresent,       // (input as Value) as ValueOptional
    kValueSetAdd,                   // (lhs as ValueSet, rhs as Value) as ValueSet
    kValueSetBuilderAdd,            // (builder as ValueSetBuilder, value as Value)
    kValueSetBuilderEnd,            // (builder as ValueSetBuilder) as ValueSet
    kValueSetBuilderNew,            // () as ValueSetBuilder
    kValueSetNew,                   // () as ValueSet
    kValueSetRemove,                // (lhs as ValueSet, rhs as Value) as ValueSet
    kValueSetUnion,                 // (lhs as Set of T, rhs as Set of T) as Set of T
    kValueToObjectMapNew,           // () as ValueToObjectMap
    kValueToObjectMapSet,           // (input as ValueToObjectMap, key as Value, value as Object) as ValueToObjectMap
    kValueToObjectMapBuilderAdd,    // (builder as ValueToObjectMapBuilder, key as Value, value as Object)
    kValueToObjectMapBuilderEnd,    // (builder as ValueToObjectMapBuilder) as ValueToObjectMap
    kValueToObjectMapBuilderNew,    // () as ValueToObjectMapBuilder
    kValueToValueMapNew,            // () as ValueToValueMap
    kValueToValueMapSet,            // (input as ValueToValueMap, key as Value, value as Value) as ValueToValueMap
    kValueToValueMapBuilderAdd,     // (builder as ValueToValueMapBuilder, key as Value, value as Value)
    kValueToValueMapBuilderEnd,     // (builder as ValueToValueMapBuilder) as ValueToValueMap
    kValueToValueMapBuilderNew,     // () as ValueToValueMapBuilder
    kWriteFileBytes,                // (filePath as String, bytes as List of Number)
    kWriteFileLines,                // (filePath as String, lines as List of String)
    kWriteFileText,                 // (filePath as String, text as String)

    // Not a real system call. Used to get the number of system calls.
    kMaxSystemCall,
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

    inline const Value& getValue(const int vsiOffset) const {
        assert(vsiOffset < 0);
        assert(valueStackIndex + vsiOffset >= 0);
        return valueStack->at(valueStackIndex + vsiOffset);
    }

    inline Object* getObject(const int osiOffset) const {
        assert(osiOffset < 0);
        assert(objectStackIndex + osiOffset >= 0);
        const auto& ptr = objectStack->at(objectStackIndex + osiOffset);
        assert(ptr != nullptr);
        return ptr.get();
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
