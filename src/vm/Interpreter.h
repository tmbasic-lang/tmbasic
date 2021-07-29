#pragma once

#include "../common.h"
#include "vm/Program.h"
#include "vm/Value.h"

namespace vm {

class InterpreterError {
   public:
    vm::Value code;
    std::string message;
};

class Interpreter {
   public:
    Interpreter(Program* program, std::istream* consoleInputStream, std::ostream* consoleOutputStream);
    ~Interpreter();
    void init(int procedureIndex);
    bool run(int maxCycles);
    std::optional<InterpreterError> getError() const;

   private:
    class InterpreterPrivate* _private;
};

}  // namespace vm
