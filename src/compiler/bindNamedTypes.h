#pragma once

#include "../common.h"
#include "compiler/CompilerResult.h"
#include "compiler/ast.h"
#include "shared/vm/Program.h"

namespace compiler {

CompilerResult bindNamedTypes(ProcedureNode* procedure, const vm::Program& program);

}
