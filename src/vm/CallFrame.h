#pragma once

#include "common.h"
#include "Procedure.h"

namespace vm {

class CallFrame {
   public:
    const Procedure* const procedure;
    const uint8_t* const instruction;
    const int valueStackIndex;
    const int objectStackIndex;
    CallFrame(const Procedure* procedure, const uint8_t* instruction, int valueStackIndex, int objectStackIndex);
};

}  // namespace vm
