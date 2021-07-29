#pragma once

#include "../common.h"
#include "ast.h"
#include "vm/Program.h"

namespace compiler {

class CompiledGlobalVariable {
   public:
    std::string lowercaseName;
    bool isValue;
    int index;
    boost::local_shared_ptr<TypeNode> type;
};

class CompiledUserTypeField {
   public:
    std::string nameLowercase = "";
    std::string name = "";
    bool isValue = false;
    bool isObject = false;
    int fieldIndex = -1;
    boost::local_shared_ptr<TypeNode> type;
};

class CompiledUserType {
   public:
    std::string nameLowercase;
    std::string name;
    std::vector<std::unique_ptr<CompiledUserTypeField>> fields;
    std::unordered_map<std::string, CompiledUserTypeField*> fieldsByNameLowercase;
};

class CompiledProcedure {
   public:
    std::string nameLowercase;
    std::string name;
    size_t sourceMemberIndex;
    size_t procedureIndex;
    std::unique_ptr<ProcedureNode> procedureNode;
};

class CompiledProgram {
   public:
    vm::Program vmProgram;
    std::vector<std::unique_ptr<CompiledGlobalVariable>> globalVariables;
    std::vector<std::unique_ptr<CompiledProcedure>> procedures;

    // user types
    std::vector<std::unique_ptr<CompiledUserType>> userTypes;
    std::unordered_map<std::string, CompiledUserType*> userTypesByNameLowercase;
    std::unordered_map<size_t, CompiledUserType*> userTypesBySourceMemberIndex;
};

}  // namespace compiler
