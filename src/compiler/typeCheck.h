#pragma once

#include "../common.h"
#include "ast.h"
#include "BuiltInProcedureList.h"
#include "CompiledProgram.h"

namespace compiler {

void typeCheck(
    ProcedureNode* procedureNode,
    const SourceProgram& sourceProgram,
    CompiledProgram* compiledProgram,
    const BuiltInProcedureList& builtInProcedures);

};  // namespace compiler
