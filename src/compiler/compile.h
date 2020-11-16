#pragma once

#include "../common.h"
#include "compiler/CompilerResult.h"
#include "compiler/parse.h"
#include "shared/basic/Token.h"
#include "shared/vm/Procedure.h"
#include "shared/vm/Program.h"

namespace compiler {

CompilerResult compileProcedure(vm::Procedure* procedure, vm::Program* program);

}  // namespace compiler
