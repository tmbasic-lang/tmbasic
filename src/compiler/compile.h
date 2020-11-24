#pragma once

#include "../common.h"
#include "compiler/CompilerResult.h"
#include "shared/vm/Procedure.h"
#include "shared/vm/Program.h"

namespace compiler {

CompilerResult compile(vm::Procedure* procedure, vm::Program* program);

}  // namespace compiler
