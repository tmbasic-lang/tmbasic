#include "CallFrame.h"

namespace vm {

CallFrame::CallFrame(
    const Procedure* procedure,
    size_t instructionIndex,
    int numValueArgs,
    int numObjectArgs,
    int valueStackIndex,
    int objectStackIndex,
    bool returnsValue,
    bool returnsObject)
    : procedure(procedure),
      instructionIndex(instructionIndex),
      numValueArgs(numValueArgs),
      numObjectArgs(numObjectArgs),
      vsiArgsStart(valueStackIndex - numValueArgs),
      osiArgsStart(objectStackIndex - numObjectArgs),
      vsiLocalsStart(valueStackIndex),
      osiLocalsStart(objectStackIndex),
      returnsValue(returnsValue),
      returnsObject(returnsObject) {}

}  // namespace vm
