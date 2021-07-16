#pragma once

#include "../common.h"
#include "ast.h"

namespace compiler {

class BuiltInProcedureList {
   private:
    std::vector<std::unique_ptr<ProcedureNode>> _empty{};
    std::unordered_map<std::string, std::unique_ptr<std::vector<std::unique_ptr<ProcedureNode>>>> _map{};

    std::vector<std::unique_ptr<ProcedureNode>>* findOrCreateVector(const std::string& lowercaseName);

    ProcedureNode* addSub(
        std::string name,
        std::initializer_list<std::string> parameterNames,
        std::initializer_list<boost::local_shared_ptr<TypeNode>> parameterTypes,
        vm::SystemCall systemCall);

    ProcedureNode* addFunction(
        std::string name,
        std::initializer_list<std::string> parameterNames,
        std::initializer_list<boost::local_shared_ptr<TypeNode>> parameterTypes,
        boost::local_shared_ptr<TypeNode> returnType,
        vm::SystemCall systemCall);

   public:
    BuiltInProcedureList();
    const std::vector<std::unique_ptr<ProcedureNode>>& get(const std::string& name);
};

}  // namespace compiler
