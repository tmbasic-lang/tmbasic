#pragma once

#include "../common.h"
#include "compiler/CompilerResult.h"
#include "compiler/ast.h"
#include "vm/Program.h"

namespace compiler {

CompilerResult bindNamedTypes(ProcedureNode* procedure, const CompiledProgram& program);

}
