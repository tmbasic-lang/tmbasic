#pragma once

#include "../common.h"
#include "compiler/CompiledProgram.h"
#include "compiler/CompilerResult.h"
#include "compiler/SourceProgram.h"

namespace compiler {

// precondition: all global variables must be compiled first
CompilerResult compileProcedure(const SourceMember& sourceMember, CompiledProgram* compiledProgram);

}  // namespace compiler
