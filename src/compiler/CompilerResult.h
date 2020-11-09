#pragma once

#include "common.h"
#include "shared/basic/Token.h"

namespace compiler {

class CompilerResult {
   public:
    bool isSuccess;
    std::string message;
    basic::Token token;

    static CompilerResult success();
    static CompilerResult error(std::string message, basic::Token token);

   private:
    CompilerResult(bool isSuccess, std::string message, basic::Token token);
};

}  // namespace compiler
