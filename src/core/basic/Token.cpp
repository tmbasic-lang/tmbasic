#include "Token.h"

using namespace basic;

Token::Token() : lineIndex(0), columnIndex(-1), type(TokenKind::kEndOfFile), text({}) {}

Token::Token(int lineIndex, int columnIndex, TokenKind type, std::string text)
    : lineIndex(lineIndex), columnIndex(columnIndex), type(type), text(text) {}

Token::Token(const Token& other)
    : lineIndex(other.lineIndex), columnIndex(other.columnIndex), type(other.type), text(other.text) {}

Token::Token(Token&& other)
    : lineIndex(other.lineIndex), columnIndex(other.columnIndex), type(other.type), text(std::move(other.text)) {}

void Token::operator=(Token&& other) {
    lineIndex = other.lineIndex;
    columnIndex = other.columnIndex;
    type = other.type;
    text = std::move(other.text);
}
