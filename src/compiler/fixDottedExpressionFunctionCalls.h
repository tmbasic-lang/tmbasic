#pragma once

#include "../common.h"
#include "ast.h"
#include "BuiltInProcedureList.h"
#include "CompiledProgram.h"

namespace compiler {

void fixDottedExpressionFunctionCalls(
    ProcedureNode* procedureNode,
    const BuiltInProcedureList& builtInProcedures,
    const CompiledProgram& compiledProgram);

}  // namespace compiler
