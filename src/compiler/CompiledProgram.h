#pragma once

#include "../common.h"
#include "ast.h"
#include "shared/ObjectType.h"

namespace compiler {

class CompiledGlobalVariable {
   public:
    std::string lowercaseName;
    bool isValue;
    int index;
    std::unique_ptr<Node> dimOrConstStatementNode;
};

class CompiledUserTypeField {
   public:
    std::string nameLowercase = "";
    std::string name = "";
    bool isValue = false;
    bool isObject = false;
    int fieldIndex = -1;
    boost::local_shared_ptr<ParameterNode> parameterNode;
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
    // This first bit becomes the Program class when it gets deserialized.
    size_t vmStartupProcedureIndex = 0;
    std::vector<std::vector<uint8_t>> vmProcedures;
    std::vector<decimal::Decimal> vmGlobalValues;
    std::vector<std::pair<shared::ObjectType, std::string>> vmGlobalObjects;

    // The remained of this class is metadata that is not serialized.
    std::vector<std::unique_ptr<CompiledGlobalVariable>> globalVariables;
    std::vector<std::unique_ptr<CompiledProcedure>> procedures;

    // User types.
    std::vector<std::unique_ptr<CompiledUserType>> userTypes;
    std::unordered_map<std::string, CompiledUserType*> userTypesByNameLowercase;
    std::unordered_map<size_t, CompiledUserType*> userTypesBySourceMemberIndex;

    std::vector<uint8_t> serialize() const;
};

}  // namespace compiler
