#pragma once

#include "../common.h"
#include "compiler/CompiledProgram.h"
#include "compiler/CompilerResult.h"
#include "compiler/ast.h"

namespace compiler {

// all global variables must be in the compiled program first
CompilerResult bindProcedureSymbols(ProcedureNode* procedure, const CompiledProgram& program);

}  // namespace compiler
