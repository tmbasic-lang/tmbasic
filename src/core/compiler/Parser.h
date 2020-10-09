#pragma once

#include "common.h"
#include "core/basic/Ast.h"

namespace compiler {

class ProductionCollection;

class Parser {
   public:
    Parser();
    std::unique_ptr<basic::ProcedureNode> parseProcedure(const std::vector<basic::Token>& tokens);

   private:
    std::unique_ptr<ProductionCollection> _productionCollection;
};

}  // namespace compiler
