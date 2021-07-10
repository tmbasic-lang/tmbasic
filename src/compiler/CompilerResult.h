#pragma once

#include "../common.h"
#include "compiler/Token.h"

namespace compiler {

class CompilerResult {
   public:
    bool isSuccess;
    std::string message;
    Token token;

    static inline CompilerResult success() {
        return CompilerResult(true, std::string(), Token(0, 0, TokenKind::kEndOfFile, std::string(), nullptr));
    }
    static inline CompilerResult error(std::string message, Token token) {
        return CompilerResult(false, std::move(message), std::move(token));
    }

   private:
    inline CompilerResult(bool isSuccess, std::string message, Token token)
        : isSuccess(isSuccess), message(std::move(message)), token(std::move(token)) {}
};

}  // namespace compiler
