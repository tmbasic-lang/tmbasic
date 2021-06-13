#include "compiler/tokenize.h"

namespace compiler {

class Scanner {
   public:
    static std::vector<Token> tokenize(std::string_view input);

   private:
    std::vector<Token> _tokens;
    bool _currentTokenIsString = false;
    bool _currentTokenIsComment = false;
    bool _skipNext = false;
    std::ostringstream _currentTokenText;
    int _currentTokenColumnIndex = -1;
    int _lineIndex = 0;
    int _columnIndex = -1;
    const std::regex _integerRegex = std::regex("^[-]?[0-9]+$");
    const std::regex _numberRegex = std::regex("^[-]?[0-9]+(\\.[0-9]+)?$");
    const std::regex _identifierRegex = std::regex("^[A-Za-z][A-Za-z0-9_]*$");

    Scanner();
    void processChar(char ch, char peek);
    void append(char ch);
    void endCurrentToken();
    bool isCurrentTokenTextEmpty();
    TokenKind classifyToken(const std::string& text);
};

Scanner::Scanner() = default;

static bool isCommentToken(const Token& x) {
    return x.type == TokenKind::kComment;
}

static void removeComments(std::vector<Token>* tokens) {
    tokens->erase(std::remove_if(tokens->begin(), tokens->end(), isCommentToken));
}

static void removeBlankLines(std::vector<Token>* tokens) {
    for (auto i = tokens->size() - 1; i >= 1; i--) {
        if ((*tokens)[i].type == TokenKind::kEndOfLine && (*tokens)[i - 1].type == TokenKind::kEndOfLine) {
            tokens->erase(tokens->begin() + i);
        }
    }
}

std::vector<Token> tokenize(std::string_view input, TokenizeType type) {
    auto tokens = Scanner::tokenize(input);
    if (type == TokenizeType::kCompile) {
        removeComments(&tokens);
        removeBlankLines(&tokens);
    }
    return tokens;
}

std::vector<Token> Scanner::tokenize(std::string_view input) {
    Scanner scanner;

    auto previousChar = '\0';
    for (auto ch : input) {
        if (previousChar != '\0') {
            scanner.processChar(previousChar, ch);
        }
        previousChar = ch;
    }
    if (previousChar != '\0') {
        scanner.processChar(previousChar, '\0');
    }

    scanner.endCurrentToken();
    return scanner._tokens;
}

void Scanner::processChar(char ch, char peek) {
    _columnIndex++;
    if (_skipNext) {
        _skipNext = false;
    } else if (_currentTokenIsComment) {
        if (ch == '\n') {
            // the comment and the newline are two separate tokens
            endCurrentToken();
            append(ch);
            endCurrentToken();
            _currentTokenIsComment = false;
        } else {
            append(ch);
        }
    } else if (_currentTokenIsString) {
        append(ch);
        if (ch == '"') {
            if (peek == '"') {
                // "" is how quotes are escaped inside string literals
                _skipNext = true;
            } else {
                endCurrentToken();  // no whitespace necessary after a string literal
                _currentTokenIsString = false;
            }
        }
    } else if (ch == '\'') {
        endCurrentToken();  // no whitespace necessary before a comment
        _currentTokenIsComment = true;
        append(ch);
    } else if (ch == '"') {
        endCurrentToken();  // no whitespace necessary before a string literal
        _currentTokenIsString = true;
        append(ch);
    } else {
        switch (ch) {
            // eat whitespace
            case ' ':
            case '\t':
                endCurrentToken();
                break;

            // these characters are their own tokens and don't require whitespace on either side
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case ':':
            case '|':
            case ',':
            case '+':
            case '*':
            case '/':
            case '=':
                endCurrentToken();
                append(ch);
                endCurrentToken();
                break;

            case '\n':
                endCurrentToken();
                append(ch);
                endCurrentToken();
                _lineIndex++;
                _columnIndex = -1;  // will be incremented on the next char
                break;

            // could be start of a literal or minus operator
            case '-':
                if (peek >= '0' && peek <= '9') {
                    // start of an integer or decimal literal
                    endCurrentToken();
                    append(ch);
                } else {
                    // minus sign
                    endCurrentToken();
                    append(ch);
                    endCurrentToken();
                }
                break;

            // some lookahead logic needed to decode these operators
            case '>':
            case '<':
                endCurrentToken();
                append(ch);
                if (peek == '=') {
                    // <= >=
                    append(peek);
                    _skipNext = true;
                } else if (ch == '<' && peek == '>') {
                    // <>
                    append(peek);
                    _skipNext = true;
                }
                endCurrentToken();
                break;

            // a period could be a dot token or part of a decimal literal
            case '.':
                if (!isCurrentTokenTextEmpty()) {
                    if (std::regex_match(_currentTokenText.str(), _integerRegex)) {
                        // part of a decimal literal
                        append(ch);
                    } else {
                        // dot token
                        endCurrentToken();
                        append(ch);
                        endCurrentToken();
                    }
                } else if (isdigit(peek) != 0) {
                    // start of a decimal literal
                    append(ch);
                } else {
                    // dot token
                    endCurrentToken();
                    append(ch);
                    endCurrentToken();
                }
                break;

            default:
                append(ch);
                break;
        }
    }
}

void Scanner::append(char ch) {
    if (isCurrentTokenTextEmpty()) {
        // starting a new token
        _currentTokenColumnIndex = _columnIndex;
    }

    _currentTokenText << ch;
}

void Scanner::endCurrentToken() {
    if (!isCurrentTokenTextEmpty()) {
        auto text = _currentTokenText.str();
        auto type = classifyToken(text);
        _tokens.emplace_back(_lineIndex, _currentTokenColumnIndex, type, std::move(text));
        _currentTokenText = std::ostringstream();
    }
}

bool Scanner::isCurrentTokenTextEmpty() {
    return _currentTokenText.tellp() <= 0;
}

TokenKind Scanner::classifyToken(const std::string& text) {
    assert(text.length() > 0);
    switch (text[0]) {
        case '\n':
            return TokenKind::kEndOfLine;
        case '"':
            return TokenKind::kStringLiteral;
        case '(':
            return TokenKind::kLeftParenthesis;
        case ')':
            return TokenKind::kRightParenthesis;
        case '[':
            return TokenKind::kLeftBracket;
        case ']':
            return TokenKind::kRightBracket;
        case '{':
            return TokenKind::kLeftBrace;
        case '}':
            return TokenKind::kRightBrace;
        case ':':
            return TokenKind::kColon;
        case ',':
            return TokenKind::kComma;
        case '+':
            return TokenKind::kPlusSign;
        case '*':
            return TokenKind::kMultiplicationSign;
        case '/':
            return TokenKind::kDivisionSign;
        case '=':
            return TokenKind::kEqualsSign;
        case '\'':
            return TokenKind::kComment;

        case '<':
            if (text == "<") {
                return TokenKind::kLessThanSign;
            } else if (text == "<=") {
                return TokenKind::kLessThanEqualsSign;
            } else if (text == "<>") {
                return TokenKind::kNotEqualsSign;
            } else {
                return TokenKind::kError;
            }

        case '>':
            if (text == ">") {
                return TokenKind::kGreaterThanSign;
            } else if (text == ">=") {
                return TokenKind::kGreaterThanEqualsSign;
            } else {
                return TokenKind::kError;
            }

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
        case '.':
            if (text == "-") {
                return TokenKind::kMinusSign;
            } else if (text == ".") {
                return TokenKind::kDot;
            } else if (std::regex_match(text, _numberRegex)) {
                return TokenKind::kNumberLiteral;
            } else {
                return TokenKind::kError;
            }
    }

    if (std::regex_match(text, _identifierRegex)) {
        auto lc = boost::algorithm::to_lower_copy(text);
        switch (lc[0]) {
            case 'a':
                if (lc == "and") {
                    return TokenKind::kAnd;
                } else if (lc == "as") {
                    return TokenKind::kAs;
                }
                break;
            case 'b':
                if (lc == "boolean") {
                    return TokenKind::kBoolean;
                } else if (lc == "by") {
                    return TokenKind::kBy;
                }
                break;
            case 'c':
                if (lc == "case") {
                    return TokenKind::kCase;
                } else if (lc == "catch") {
                    return TokenKind::kCatch;
                } else if (lc == "const") {
                    return TokenKind::kConst;
                } else if (lc == "continue") {
                    return TokenKind::kContinue;
                }
                break;
            case 'd':
                if (lc == "date") {
                    return TokenKind::kDate;
                } else if (lc == "datetime") {
                    return TokenKind::kDateTime;
                } else if (lc == "datetimeoffset") {
                    return TokenKind::kDateTimeOffset;
                } else if (lc == "dim") {
                    return TokenKind::kDim;
                } else if (lc == "do") {
                    return TokenKind::kDo;
                }
                break;
            case 'e':
                if (lc == "each") {
                    return TokenKind::kEach;
                } else if (lc == "else") {
                    return TokenKind::kElse;
                } else if (lc == "end") {
                    return TokenKind::kEnd;
                } else if (lc == "exit") {
                    return TokenKind::kExit;
                }
                break;
            case 'f':
                if (lc == "false") {
                    return TokenKind::kFalse;
                } else if (lc == "finally") {
                    return TokenKind::kFinally;
                } else if (lc == "for") {
                    return TokenKind::kFor;
                } else if (lc == "from") {
                    return TokenKind::kFrom;
                } else if (lc == "function") {
                    return TokenKind::kFunction;
                }
                break;
            case 'g':
                if (lc == "group") {
                    return TokenKind::kGroup;
                }
                break;
            case 'i':
                if (lc == "if") {
                    return TokenKind::kIf;
                } else if (lc == "in") {
                    return TokenKind::kIn;
                } else if (lc == "into") {
                    return TokenKind::kInto;
                }
                break;
            case 'j':
                if (lc == "join") {
                    return TokenKind::kJoin;
                }
                break;
            case 'k':
                if (lc == "key") {
                    return TokenKind::kKey;
                }
                break;
            case 'l':
                if (lc == "list") {
                    return TokenKind::kList;
                } else if (lc == "loop") {
                    return TokenKind::kLoop;
                }
                break;
            case 'm':
                if (lc == "map") {
                    return TokenKind::kMap;
                } else if (lc == "mod") {
                    return TokenKind::kMod;
                }
                break;
            case 'n':
                if (lc == "next") {
                    return TokenKind::kNext;
                } else if (lc == "not") {
                    return TokenKind::kNot;
                } else if (lc == "number") {
                    return TokenKind::kNumber;
                }
                break;
            case 'o':
                if (lc == "of") {
                    return TokenKind::kOf;
                } else if (lc == "on") {
                    return TokenKind::kOn;
                } else if (lc == "optional") {
                    return TokenKind::kOptional;
                } else if (lc == "or") {
                    return TokenKind::kOr;
                }
                break;
            case 'r':
                if (lc == "record") {
                    return TokenKind::kRecord;
                } else if (lc == "rethrow") {
                    return TokenKind::kRethrow;
                } else if (lc == "return") {
                    return TokenKind::kReturn;
                }
                break;
            case 's':
                if (lc == "select") {
                    return TokenKind::kSelect;
                } else if (lc == "step") {
                    return TokenKind::kStep;
                } else if (lc == "sub") {
                    return TokenKind::kSub;
                }
                break;
            case 't':
                if (lc == "text") {
                    return TokenKind::kString;
                } else if (lc == "then") {
                    return TokenKind::kThen;
                } else if (lc == "throw") {
                    return TokenKind::kThrow;
                } else if (lc == "timespan") {
                    return TokenKind::kTimeSpan;
                } else if (lc == "timezone") {
                    return TokenKind::kTimeZone;
                } else if (lc == "to") {
                    return TokenKind::kTo;
                } else if (lc == "true") {
                    return TokenKind::kTrue;
                } else if (lc == "try") {
                    return TokenKind::kTry;
                } else if (lc == "type") {
                    return TokenKind::kType;
                }
                break;
            case 'u':
                if (lc == "until") {
                    return TokenKind::kUntil;
                }
                break;
            case 'w':
                if (lc == "wend") {
                    return TokenKind::kWend;
                } else if (lc == "where") {
                    return TokenKind::kWhere;
                } else if (lc == "while") {
                    return TokenKind::kWhile;
                } else if (lc == "with") {
                    return TokenKind::kWith;
                }
                break;
        }
        return TokenKind::kIdentifier;
    }
    return TokenKind::kError;
}

}  // namespace compiler
