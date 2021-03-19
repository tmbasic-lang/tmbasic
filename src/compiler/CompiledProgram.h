#pragma once

#include "../common.h"
#include "vm/Program.h"

namespace compiler {

class FieldNode;
class TypeNode;

class CompiledGlobalVariable {
   public:
    std::string lowercaseName;
    bool isValue;
    int index;
    boost::local_shared_ptr<TypeNode> type;
};

class CompiledNamedType {
   public:
    std::string lowercaseName;
    std::vector<boost::local_shared_ptr<FieldNode>> fields;
};

class CompiledProgram {
   public:
    vm::Program vmProgram;
    std::vector<std::unique_ptr<CompiledGlobalVariable>> globalVariables;
    std::vector<std::unique_ptr<CompiledNamedType>> namedTypes;
};

}  // namespace compiler
