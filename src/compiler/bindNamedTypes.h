#pragma once

#include "common.h"
#include "CompilerResult.h"
#include "shared/basic/ast.h"
#include "shared/vm/Program.h"

namespace compiler {

CompilerResult bindNamedTypes(basic::ProcedureNode& procedure, const vm::Program& program);

}
