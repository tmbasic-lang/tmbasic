#pragma once

#include "common.h"
#include "core/basic/Ast.h"
#include "core/vm/Program.h"
#include "CompilerResult.h"

namespace compiler {

CompilerResult bindNamedTypes(basic::ProcedureNode& procedure, const vm::Program& program);

}
