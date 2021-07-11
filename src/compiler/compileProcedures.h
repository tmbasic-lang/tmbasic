#pragma once

#include "../common.h"
#include "compiler/CompiledProgram.h"
#include "compiler/SourceProgram.h"

namespace compiler {

void compileProcedures(const SourceProgram& sourceProgram, CompiledProgram* compiledProgram);

}  // namespace compiler
