#pragma once

#include "common.h"
#include "shared/basic/ast.h"

namespace compiler {

class ParserResult {
   public:
    bool isSuccess;
    std::string message;                // if !success
    std::optional<basic::Token> token;  // if !success
    std::unique_ptr<basic::Node> node;  // if success

    ParserResult(std::string message, basic::Token token);
    ParserResult(std::unique_ptr<basic::Node> node);
};

enum class ParserRootProduction { kProgram, kMember };

ParserResult parse(ParserRootProduction rootProduction, const std::vector<basic::Token>& tokens);

}  // namespace compiler
