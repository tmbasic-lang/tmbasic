#pragma once

#include "common.h"
#include "basic/tokens/Token.h"
#include "basic/tokens/TokenType.h"

namespace compiler {

class Scanner {
   public:
    static std::vector<basic::Token> tokenize(const std::string& input);

   private:
    std::vector<basic::Token> _tokens;
    bool _currentTokenIsString = false;
    bool _skipNext = false;
    std::ostringstream _currentTokenText;
    int _currentTokenColumnIndex = -1;
    int _lineIndex = 0;
    int _columnIndex = -1;
    const std::regex _integerRegex = std::regex("^[-]?[0-9]+$");
    const std::regex _decimalRegex = std::regex("^[-]?[0-9]+(\\.[0-9]+)?$");
    const std::regex _identifierRegex = std::regex("^[A-Za-z][A-Za-z0-9_]*$");

    Scanner();
    void processChar(char ch, char peek);
    void append(char ch);
    void endCurrentToken();
    bool isCurrentTokenTextEmpty();
    basic::TokenType classifyToken(const std::string& text);
};

}  // namespace compiler
