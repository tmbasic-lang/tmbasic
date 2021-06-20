#pragma once

#include "../common.h"
#include "compiler/ParserResult.h"
#include "compiler/SourceProgram.h"
#include "compiler/ast.h"

namespace compiler {

enum class ParserRootProduction { kProgram, kMember };

ParserResult parse(
    const SourceMember* sourceMember,
    ParserRootProduction rootProduction,
    const std::vector<Token>& tokens);

}  // namespace compiler
