#pragma once

#include "../common.h"
#include "compiler/ast.h"
#include "vm/Program.h"

namespace compiler {

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
