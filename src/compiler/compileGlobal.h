#pragma once

#include "../common.h"
#include "compiler/CompiledProgram.h"
#include "compiler/CompilerResult.h"
#include "compiler/SourceProgram.h"

namespace compiler {

CompilerResult compileGlobal(const SourceMember& sourceMember, CompiledProgram* compiledProgram);

}  // namespace compiler
