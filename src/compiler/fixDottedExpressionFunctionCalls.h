#pragma once

#include "../common.h"
#include "compiler/CompiledProgram.h"
#include "compiler/ast.h"

namespace compiler {

void fixDottedExpressionFunctionCalls(
    ProcedureNode* procedureNode,
    const BuiltInProcedureList& builtInProcedures,
    const CompiledProgram& compiledProgram);

}  // namespace compiler
