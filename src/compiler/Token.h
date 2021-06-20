#pragma once

#include "../common.h"
#include "TokenKind.h"
#include "SourceProgram.h"

namespace compiler {

class Token {
   public:
    int lineIndex = 0;
    int columnIndex = -1;
    TokenKind type = TokenKind::kEndOfFile;
    std::string text = "";
    const SourceMember* sourceMember = nullptr;

    Token();
    ~Token();
    Token(int lineIndex, int columnIndex, TokenKind type, std::string text, const SourceMember* sourceMember);
    Token(const Token& other);
    Token(Token&& other) noexcept;
    Token& operator=(Token&& other) noexcept;
    Token& operator=(const Token&) = delete;
};

}  // namespace compiler
