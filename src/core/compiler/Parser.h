#pragma once

#include "common.h"
#include "core/basic/Ast.h"

namespace compiler {

class ProductionCollection;

class ParserResult {
   public:
    bool success;
    std::string message;                // if !success
    std::optional<basic::Token> token;  // if !success
    std::unique_ptr<basic::Node> node;  // if success

    ParserResult(std::string message, basic::Token token);
    ParserResult(std::unique_ptr<basic::Node> node);
};

class Parser {
   public:
    Parser();
    ParserResult parseProgram(const std::vector<basic::Token>& tokens);
    ParserResult parseMember(const std::vector<basic::Token>& tokens);

   private:
    std::unique_ptr<ProductionCollection> _productionCollection;
};

}  // namespace compiler
