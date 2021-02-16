#pragma once

#include "../common.h"
#include "vm/Procedure.h"

namespace vm {

class CallFrame {
   public:
    const Procedure* const procedure;
    const size_t instructionIndex;
    const int valueStackIndex;
    const int objectStackIndex;
    CallFrame(const Procedure* procedure, size_t instructionIndex, int valueStackIndex, int objectStackIndex);
};

}  // namespace vm
