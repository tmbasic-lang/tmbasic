#pragma once

#include "../common.h"
#include "compiler/Token.h"

namespace compiler {

std::vector<Token> tokenize(const std::string& input);

}  // namespace compiler
