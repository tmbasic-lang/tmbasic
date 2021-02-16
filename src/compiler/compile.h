#pragma once

#include "../common.h"
#include "compiler/CompilerResult.h"
#include "compiler/SourceProgram.h"

namespace compiler {

CompilerResult compile(const SourceMember& member, const SourceProgram& program);

}  // namespace compiler
