#pragma once

#include "../common.h"
#include "vm/Program.h"

namespace compiler {

class TypeNode;

class CompiledGlobalVariable {
   public:
    std::string lowercaseName;
    bool isValue;
    uint16_t index;
    std::unique_ptr<TypeNode> type;
};

class CompiledProgram {
   public:
    vm::Program vmProgram;
    std::vector<CompiledGlobalVariable> globalVariables;
};

}  // namespace compiler
