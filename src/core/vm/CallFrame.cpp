#include "CallFrame.h"

namespace vm {

CallFrame::CallFrame(const Procedure* procedure, const uint8_t* instruction, int valueStackIndex, int objectStackIndex)
    : procedure(procedure),
      instruction(instruction),
      valueStackIndex(valueStackIndex),
      objectStackIndex(objectStackIndex) {}

}  // namespace vm
