#pragma once

#include "../common.h"
#include "compiler/CompilerResult.h"
#include "compiler/SourceProgram.h"
#include "compiler/ast.h"

namespace compiler {

CompilerResult bindProcedureSymbols(ProcedureNode* procedure, const SourceProgram& program);

}
