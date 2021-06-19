#pragma once

#include "../common.h"
#include "compiler/CompiledProgram.h"
#include "compiler/CompilerResult.h"
#include "compiler/SourceProgram.h"

namespace compiler {

CompilerResult compileProgram(const SourceProgram& sourceProgram, CompiledProgram* compiledProgram);

}  // namespace compiler
