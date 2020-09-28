#pragma once

#include "common.h"
#include "CallFrame.h"
#include "Object.h"
#include "Program.h"
#include "RecordBuilder.h"
#include "Value.h"
#include "List.h"

namespace vm {

const int kValueStackSize = 10000;
const int kObjectStackSize = 10000;

class Interpreter {
   public:
    Interpreter(const Program& program);
    void init(int procedureIndex);
    bool run(int maxCycles);

   private:
    const Program& _program;

    std::stack<CallFrame> _callStack;
    Value _valueStack[kValueStackSize];
    boost::local_shared_ptr<Object> _objectStack[kObjectStackSize];
    std::stack<RecordBuilder> _recordBuilderStack;
    std::stack<ObjectListBuilder> _objectListBuilderStack;
    std::stack<ValueListBuilder> _valueListBuilderStack;
    bool _hasError;
    boost::local_shared_ptr<Object> _errorMessage;
    Value _errorCode;

    // these are a snapshot that is only updated when run() returns
    const Procedure* _procedure;
    const uint8_t* _instruction;
    Value _a, _b;
    boost::local_shared_ptr<Object> _x, _y, _z;
    int _valueStackIndex;
    int _objectStackIndex;

    class ReturnResult {
       public:
        const Procedure* const procedure;
        const std::vector<uint8_t>& instructions;
        const uint8_t* const pc;
        const int valueStackIndex;
        const int objectStackIndex;
        ReturnResult(
            const Procedure* procedure,
            const std::vector<uint8_t>& instructions,
            const uint8_t* pc,
            int valueStackIndex,
            int objectStackIndex);
    };

    ReturnResult returnFromProcedure(int valueStackIndex, int objectStackIndex);
};

}  // namespace vm
