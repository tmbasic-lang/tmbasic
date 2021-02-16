#pragma once

#include "../common.h"
#include "compiler/ast.h"

namespace compiler {

class ParserResult {
   public:
    bool isSuccess;
    std::string message;         // if !success
    std::optional<Token> token;  // if !success
    std::unique_ptr<Node> node;  // if success

    ParserResult(std::string message, Token token);
    explicit ParserResult(std::unique_ptr<Node> node);
};

}  // namespace compiler
