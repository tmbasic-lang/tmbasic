#pragma once

#include "common.h"
#include "core/basic/Token.h"
#include "core/vm/Procedure.h"
#include "core/vm/Program.h"
#include "CompilerResult.h"
#include "Parser.h"

namespace compiler {

class Compiler {
   public:
    CompilerResult compileProcedure(vm::Procedure& procedure, vm::Program& program);

   private:
    Parser _parser;
};

}  // namespace compiler
