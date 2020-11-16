#pragma once

#include "../common.h"
#include "shared/basic/Token.h"

namespace compiler {

std::vector<basic::Token> tokenize(const std::string& input);

}  // namespace compiler
