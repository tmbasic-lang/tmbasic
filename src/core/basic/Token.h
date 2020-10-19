#pragma once

#include "common.h"
#include "TokenType.h"

namespace basic {

class Token {
   public:
    int lineIndex;
    int columnIndex;
    TokenType type;
    std::string text;

    Token(int lineIndex, int columnIndex, TokenType type, std::string text);
    Token(const Token& other);
    Token(Token&& other);
    void operator=(Token&& other);
};

}  // namespace basic
