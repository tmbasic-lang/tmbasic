#pragma once

#include "common.h"
#include "core/basic/Token.h"
#include "core/vm/Procedure.h"
#include "CompilerResult.h"
#include "Parser.h"

namespace compiler {

class Compiler {
   public:
    CompilerResult compile(vm::Procedure& procedure);

   private:
    Parser _parser;
};

}  // namespace compiler
