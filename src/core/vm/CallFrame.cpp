#include "CallFrame.h"

using namespace vm;

CallFrame::CallFrame(const Procedure* procedure, const uint8_t* instruction, int valueStackIndex, int objectStackIndex)
    : procedure(procedure),
      instruction(instruction),
      valueStackIndex(valueStackIndex),
      objectStackIndex(objectStackIndex) {}
