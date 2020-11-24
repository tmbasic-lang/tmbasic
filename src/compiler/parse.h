#pragma once

#include "../common.h"
#include "compiler/ParserResult.h"
#include "shared/basic/ast.h"

namespace compiler {

enum class ParserRootProduction { kProgram, kMember };

ParserResult parse(ParserRootProduction rootProduction, const std::vector<basic::Token>& tokens);

}  // namespace compiler
