#pragma once

#include "../common.h"
#include "compiler/CompilerResult.h"
#include "shared/basic/ast.h"
#include "shared/vm/Program.h"

namespace compiler {

CompilerResult bindProcedureSymbols(basic::ProcedureNode* procedure, const vm::Program& program);

}
