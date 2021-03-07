#pragma once

#include "../common.h"
#include "vm/Value.h"
#include "vm/Object.h"
#include "vm/constants.h"

namespace vm {

enum class SystemCall {
    kAvailableLocales,   // AvailableLocales() as List of String
    kCharacters1,        // Characters(input as String) as List of String
    kCharacters2,        // Characters(input as String, locale as String) as List of String
    kChr,                // Chr(input as Number) as String
    kDays,               // Days(count as Number) as TimeSpan
    kHasValueO,          // HasValue(input as Optional Object) as Boolean
    kHasValueV,          // HasValue(input as Optional Value) as Boolean
    kHours,              // Hours(count as Number) as TimeSpan
    kLen,                // Len(input as String) as Number
    kMilliseconds,       // Milliseconds(count as Number) as TimeSpan
    kMinutes,            // Minutes(count as Number) as TimeSpan
    kSeconds,            // Seconds(count as Number) as TimeSpan
    kTotalDays,          // TotalDays(span as TimeSpan) as Number
    kTotalHours,         // TotalHours(span as TimeSpan) as Number
    kTotalMilliseconds,  // TotalMilliseconds(span as TimeSpan) as Number
    kTotalMinutes,       // TotalMinutes(span as TimeSpan) as Number
    kTotalSeconds,       // TotalSeconds(span as TimeSpan) as Number
    kValueO,             // ValueO(input as Optional Object) as Boolean
    kValueV,             // ValueV(input as Optional Value) as Boolean
};

class SystemCallInput {
   public:
    const std::array<Value, kValueStackSize>& valueStack;
    const std::array<boost::local_shared_ptr<Object>, kObjectStackSize>& objectStack;
    int valueStackIndex;
    int objectStackIndex;
    SystemCallInput(
        const std::array<Value, kValueStackSize>& valueStack,
        const std::array<boost::local_shared_ptr<Object>, kObjectStackSize>& objectStack,
        int valueStackIndex,
        int objectStackIndex);
};

class SystemCallResult {
   public:
    Value a = {};
    boost::local_shared_ptr<Object> x = nullptr;
    bool hasError = false;
    std::string errorMessage = "";
    int errorCode = 0;
};

void initSystemCalls();
SystemCallResult systemCall(SystemCall which, const SystemCallInput& input);

}  // namespace vm
