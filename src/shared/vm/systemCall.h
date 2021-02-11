#pragma once

#include "../../common.h"
#include "shared/vm/Value.h"
#include "shared/vm/Object.h"
#include "shared/vm/constants.h"

namespace vm {

enum class SystemCall {
    kAvailableLocales,
    kChr,
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
    Value a;
    boost::local_shared_ptr<Object> x;
    int popValues;
    int popObjects;
    SystemCallResult(int popValues, int popObjects);
    SystemCallResult(Value a, int popValues, int popObjects);
    SystemCallResult(boost::local_shared_ptr<Object> x, int popValues, int popObjects);
};

SystemCallResult systemCall(SystemCall which, const SystemCallInput& input);

}  // namespace vm
