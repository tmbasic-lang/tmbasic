#pragma once

#include "../common.h"
#include "vm/Object.h"
#include "vm/Procedure.h"
#include "vm/Value.h"

namespace vm {

class CallFrame {
   public:
    const Procedure* const procedure;
    const size_t instructionIndex;
    const bool returnsValue;
    const bool returnsObject;

    const int numValueArgs;
    const int vsiArgsStart;

    const int numObjectArgs;
    const int osiArgsStart;

    const int vsiLocalsStart;
    const int osiLocalsStart;

    CallFrame(
        const Procedure* procedure,
        size_t instructionIndex,
        int numValueArgs,
        int numObjectArgs,
        int valueStackIndex,
        int objectStackIndex,
        bool returnsValue,
        bool returnsObject);
};

}  // namespace vm
