#pragma once

#include "../common.h"
#include "vm/Program.h"

namespace compiler {

class TypeNode;

class CompiledGlobalVariable {
   public:
    std::string lowercaseName;
    bool isValue;
    int index;
    std::unique_ptr<TypeNode> type;
};

class CompiledProgram {
   public:
    vm::Program vmProgram;
    std::vector<std::unique_ptr<CompiledGlobalVariable>> globalVariables;
};

}  // namespace compiler
