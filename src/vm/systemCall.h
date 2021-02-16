#pragma once

#include "../common.h"
#include "vm/Value.h"
#include "vm/Object.h"
#include "vm/constants.h"

namespace vm {

enum class SystemCall {
    kAvailableLocales,  // AvailableLocales() as List of String
    kCharacters1,       // Characters(input as String) as List of String
    kCharacters2,       // Characters(input as String, locale as String) as List of String
    kChr,               // Chr(input as Number) as String
    kHasValueV,         // HasValue(input as Optional Value) as Boolean
    kHasValueO,         // HasValue(input as Optional Object) as Boolean
    kLen,               // Len(input as String) as Number
    kValueV,            // ValueV(input as Optional Value) as Boolean
    kValueO,            // ValueO(input as Optional Object) as Boolean
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
    int popValues = 0;
    int popObjects = 0;
    bool hasError = false;
    std::string errorMessage = "";
    int errorCode = 0;
};

SystemCallResult systemCall(SystemCall which, const SystemCallInput& input);

}  // namespace vm
