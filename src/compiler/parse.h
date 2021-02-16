#pragma once

#include "../common.h"
#include "compiler/ParserResult.h"
#include "compiler/ast.h"

namespace compiler {

enum class ParserRootProduction { kProgram, kMember };

ParserResult parse(ParserRootProduction rootProduction, const std::vector<Token>& tokens);

}  // namespace compiler
