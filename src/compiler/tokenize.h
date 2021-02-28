#pragma once

#include "../common.h"
#include "compiler/Token.h"

namespace compiler {

enum class TokenizeType {
    kCompile, // strip comments and blank lines
    kFormat // include everything
};

std::vector<Token> tokenize(const std::string& input, TokenizeType type);

}  // namespace compiler
