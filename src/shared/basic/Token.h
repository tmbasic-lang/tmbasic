#pragma once

#include "../../common.h"
#include "shared/basic/TokenKind.h"

namespace basic {

class Token {
   public:
    int lineIndex;
    int columnIndex;
    TokenKind type;
    std::string text;

    Token();
    Token(int lineIndex, int columnIndex, TokenKind type, std::string text);
    Token(const Token& other);
    Token(Token&& other);
    void operator=(Token&& other);
};

}  // namespace basic
