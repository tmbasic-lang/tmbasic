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
    ~Interpreter();
    void init(int procedureIndex);
    bool run(int maxCycles);

   private:
    class InterpreterPrivate* _private;
};

}  // namespace vm
