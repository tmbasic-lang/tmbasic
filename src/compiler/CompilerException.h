#pragma once

#include "../common.h"
#include "compiler/Token.h"

namespace compiler {

class CompilerException : public std::exception {
   public:
    std::string message;
    Token token;
    inline CompilerException(const std::string& message, const Token& token) : message(message), token(token) {}
};

}  // namespace compiler
