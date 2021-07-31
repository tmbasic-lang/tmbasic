#pragma once

#include "../common.h"
#include "ast.h"
#include "../vm/Object.h"
#include "../vm/Value.h"

namespace compiler {

class BuiltInConstantList {
   public:
    std::unordered_map<std::string, std::unique_ptr<ConstStatementNode>> constants;

    BuiltInConstantList();

   private:
    void addNumber(std::string name, decimal::Decimal number);
    void addString(std::string name, std::string string);

    boost::local_shared_ptr<TypeNode> _numberType;
    boost::local_shared_ptr<TypeNode> _stringType;
};

}  // namespace compiler
