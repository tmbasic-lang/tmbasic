#pragma once

#include "../common.h"
#include "compiler/ast.h"

namespace compiler {

void typeCheck(ProcedureNode* procedureNode, const SourceProgram& sourceProgram, CompiledProgram* compiledProgram);

};  // namespace compiler
