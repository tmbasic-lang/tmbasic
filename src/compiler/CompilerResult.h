#pragma once

#include "../common.h"
#include "compiler/Token.h"

namespace compiler {

class CompilerResult {
   public:
    bool isSuccess;
    std::string message;
    Token token;

    static CompilerResult success();
    static CompilerResult error(std::string message, Token token);

   private:
    CompilerResult(bool isSuccess, std::string message, Token token);
};

}  // namespace compiler
