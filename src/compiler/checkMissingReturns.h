#pragma once

#include "../common.h"
#include "compiler/ast.h"

namespace compiler {

void checkMissingReturns(ProcedureNode* procedure);

}  // namespace compiler
