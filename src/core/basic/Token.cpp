#include "Token.h"

using namespace basic;

Token::Token(int lineIndex, int columnIndex, TokenType type, std::string&& text)
    : lineIndex(lineIndex), columnIndex(columnIndex), type(type), text(text) {}
