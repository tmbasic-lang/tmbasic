#pragma once

#include "common.h"
#include "shared/basic/Token.h"
#include "shared/vm/Procedure.h"
#include "shared/vm/Program.h"
#include "CompilerResult.h"
#include "parse.h"

namespace compiler {

CompilerResult compileProcedure(vm::Procedure& procedure, vm::Program& program);

}  // namespace compiler
