#pragma once

#include "../common.h"
#include "compiler/CompiledProgram.h"
#include "compiler/ast.h"

namespace compiler {

void bindNamedRecordTypes(ProcedureNode* procedureNode, const CompiledProgram& compiledProgram);

}  // namespace compiler
