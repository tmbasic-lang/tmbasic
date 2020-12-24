#pragma once

#include "../../common.h"
#include "shared/vm/CallFrame.h"
#include "shared/vm/List.h"
#include "shared/vm/Object.h"
#include "shared/vm/Program.h"
#include "shared/vm/RecordBuilder.h"
#include "shared/vm/Value.h"

namespace vm {

const int kValueStackSize = 10000;
const int kObjectStackSize = 10000;

class Interpreter {
   public:
    explicit Interpreter(const Program& program);
    void init(int procedureIndex);
    bool run(int maxCycles);

   private:
    const Program& _program;

    std::stack<CallFrame> _callStack;
    std::array<Value, kValueStackSize> _valueStack;
    std::array<boost::local_shared_ptr<Object>, kObjectStackSize> _objectStack;
    std::stack<RecordBuilder> _recordBuilderStack;
    std::stack<ObjectListBuilder> _objectListBuilderStack;
    std::stack<ValueListBuilder> _valueListBuilderStack;
    bool _hasError = false;
    boost::local_shared_ptr<Object> _errorMessage;
    Value _errorCode;

    // these are a snapshot that is only updated when run() returns
    const Procedure* _procedure = nullptr;
    size_t _instructionIndex = 0;
    Value _a, _b;
    boost::local_shared_ptr<Object> _x, _y, _z;
    int _valueStackIndex = 0;
    int _objectStackIndex = 0;

    class ReturnResult {
       public:
        const Procedure* const procedure;
        const std::vector<uint8_t>& instructions;
        const size_t instructionIndex;
        const int valueStackIndex;
        const int objectStackIndex;
        ReturnResult(
            const Procedure* procedure,
            const std::vector<uint8_t>& instructions,
            size_t instructionIndex,
            int valueStackIndex,
            int objectStackIndex);
    };

    ReturnResult returnFromProcedure(int valueStackIndex, int objectStackIndex);
};

}  // namespace vm
