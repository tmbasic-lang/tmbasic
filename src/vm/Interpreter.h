#pragma once

#include "../common.h"
#include "vm/CallFrame.h"
#include "vm/List.h"
#include "vm/Object.h"
#include "vm/Program.h"
#include "vm/RecordBuilder.h"
#include "vm/Value.h"
#include "vm/constants.h"

namespace vm {

class Interpreter {
   public:
    Interpreter(Program* program, std::istream* consoleInputStream, std::ostream* consoleOutputStream);
    void init(int procedureIndex);
    bool run(int maxCycles);

   private:
    Program* _program;
    std::istream* _consoleInputStream;
    std::ostream* _consoleOutputStream;

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
        const std::vector<uint8_t>* instructions;
        const size_t instructionIndex;
        const int valueStackIndex;
        const int objectStackIndex;
        ReturnResult(
            const Procedure* procedure,
            const std::vector<uint8_t>* instructions,
            size_t instructionIndex,
            int valueStackIndex,
            int objectStackIndex);
    };

    ReturnResult returnFromProcedure(int valueStackIndex, int objectStackIndex);
};

}  // namespace vm
