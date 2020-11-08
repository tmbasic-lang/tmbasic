#pragma once

#include "common.h"
#include "core/basic/Token.h"
#include "core/vm/Procedure.h"
#include "core/vm/Program.h"
#include "CompilerResult.h"
#include "parse.h"

namespace compiler {

CompilerResult compileProcedure(vm::Procedure& procedure, vm::Program& program);

}  // namespace compiler
