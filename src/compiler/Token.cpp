#include "Token.h"

namespace compiler {

Token::Token() = default;

Token::~Token() = default;

Token::Token(int lineIndex, int columnIndex, TokenKind type, std::string text)
    : lineIndex(lineIndex), columnIndex(columnIndex), type(type), text(std::move(text)) {}

Token::Token(const Token& other) = default;

Token::Token(Token&& other) noexcept
    : lineIndex(other.lineIndex), columnIndex(other.columnIndex), type(other.type), text(std::move(other.text)) {}

Token& Token::operator=(Token&& other) noexcept {
    lineIndex = other.lineIndex;
    columnIndex = other.columnIndex;
    type = other.type;
    text = std::move(other.text);
    return *this;
}

}  // namespace compiler
