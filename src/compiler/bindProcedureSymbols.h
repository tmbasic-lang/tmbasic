#pragma once

#include "../common.h"
#include "compiler/CompiledProgram.h"
#include "compiler/CompilerResult.h"
#include "compiler/ast.h"

namespace compiler {

CompilerResult bindProcedureSymbols(ProcedureNode* procedure, const CompiledProgram& program);

}
