#pragma once

#include "../common.h"
#include "compiler/CompilerResult.h"
#include "compiler/ast.h"

namespace compiler {

CompilerResult typeCheck(ProcedureNode* procedureNode);

};  // namespace compiler
