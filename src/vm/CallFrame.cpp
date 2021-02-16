#include "CallFrame.h"

namespace vm {

CallFrame::CallFrame(const Procedure* procedure, size_t instructionIndex, int valueStackIndex, int objectStackIndex)
    : procedure(procedure),
      instructionIndex(instructionIndex),
      valueStackIndex(valueStackIndex),
      objectStackIndex(objectStackIndex) {}

}  // namespace vm
