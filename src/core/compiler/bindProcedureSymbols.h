#pragma once

#include "common.h"
#include "core/basic/Ast.h"
#include "core/vm/Program.h"
#include "CompilerResult.h"

namespace compiler {

CompilerResult bindProcedureSymbols(basic::ProcedureNode& procedure, const vm::Program& program);

}
