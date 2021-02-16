#pragma once

#include "../common.h"
#include "compiler/CompilerResult.h"
#include "compiler/SourceProgram.h"
#include "shared/basic/ast.h"

namespace compiler {

CompilerResult bindProcedureSymbols(basic::ProcedureNode* procedure, const SourceProgram& program);

}
