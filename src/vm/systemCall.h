#pragma once

#include "../common.h"
#include "shared/SystemCalls.h"
#include "vm/constants.h"
#include "shared/Error.h"
#include "vm/Interpreter.h"
#include "vm/List.h"
#include "vm/Object.h"
#include "vm/String.h"
#include "vm/Value.h"

namespace vm {

class SystemCallInput {
   public:
    Interpreter* interpreter;
    std::array<Value, kValueStackSize>* valueStack;
    std::array<boost::intrusive_ptr<Object>, kObjectStackSize>* objectStack;
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
        std::array<boost::intrusive_ptr<Object>, kObjectStackSize>* objectStack,
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

    inline boost::intrusive_ptr<Object> getObjectPtr(const int osiOffset) const {
        return objectStack->at(objectStackIndex + osiOffset);
    }
};

class SystemCallResult {
   public:
    Value returnedValue = {};
    boost::intrusive_ptr<Object> returnedObject = nullptr;
    bool hasError = false;
    std::string errorMessage = "";
    int errorCode = 0;
};

typedef void (*SystemCallFunc)(const SystemCallInput&, SystemCallResult*);

void initSystemCall(shared::SystemCall which, SystemCallFunc func);
SystemCallResult systemCall(shared::SystemCall which, const SystemCallInput& input);

}  // namespace vm
