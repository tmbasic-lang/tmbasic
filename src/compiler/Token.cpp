#include "Token.h"

namespace compiler {

Token::Token() = default;

Token::~Token() = default;

Token::Token(int lineIndex, int columnIndex, TokenKind type, std::string text, const SourceMember* sourceMember)
    : lineIndex(lineIndex), columnIndex(columnIndex), type(type), text(std::move(text)), sourceMember(sourceMember) {}

Token::Token(const Token& other) = default;

Token::Token(Token&& other) noexcept
    : lineIndex(other.lineIndex),
      columnIndex(other.columnIndex),
      type(other.type),
      text(std::move(other.text)),
      sourceMember(other.sourceMember) {}

Token& Token::operator=(Token&& other) noexcept {
    lineIndex = other.lineIndex;
    columnIndex = other.columnIndex;
    type = other.type;
    text = std::move(other.text);
    sourceMember = other.sourceMember;
    return *this;
}

}  // namespace compiler
