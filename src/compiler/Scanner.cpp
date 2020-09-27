#include "Scanner.h"

using namespace basic;
using namespace compiler;

Scanner::Scanner() {}

std::vector<Token> Scanner::tokenize(const std::string& input) {
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

    // end of line token
    scanner._columnIndex++;
    scanner.append('\n');
    scanner.endCurrentToken();

    return scanner._tokens;
}

void Scanner::processChar(char ch, char peek) {
    _columnIndex++;
    if (_skipNext) {
        _skipNext = false;
    } else if (_currentTokenIsString) {
        append(ch);
        if (ch == '"') {
            if (peek == '"') {
                // "" is how quotes are escaped inside string literals
                _skipNext = true;
            } else {
                _currentTokenIsString = false;
            }
        }
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
                } else if (isdigit(peek)) {
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
        _tokens.push_back(Token(_lineIndex, _currentTokenColumnIndex, type, std::move(text)));
        _currentTokenText = {};
    }
}

bool Scanner::isCurrentTokenTextEmpty() {
    return _currentTokenText.tellp() <= 0;
}

TokenType Scanner::classifyToken(const std::string& text) {
    assert(text.length() > 0);
    switch (text[0]) {
        case '\n':
            return TokenType::kEndOfLine;
        case '"':
            return TokenType::kStringLiteral;
        case '(':
            return TokenType::kLeftParenthesis;
        case ')':
            return TokenType::kRightParenthesis;
        case '[':
            return TokenType::kLeftBracket;
        case ']':
            return TokenType::kRightBracket;
        case '{':
            return TokenType::kLeftBrace;
        case '}':
            return TokenType::kRightBrace;
        case ':':
            return TokenType::kColon;
        case ',':
            return TokenType::kComma;
        case '+':
            return TokenType::kPlusSign;
        case '*':
            return TokenType::kMultiplicationSign;
        case '/':
            return TokenType::kDivisionSign;
        case '=':
            return TokenType::kEqualsSign;

        case '<':
            if (text == "<") {
                return TokenType::kLessThanSign;
            } else if (text == "<=") {
                return TokenType::kLessThanEqualsSign;
            } else if (text == "<>") {
                return TokenType::kNotEqualsSign;
            } else {
                return TokenType::kError;
            }

        case '>':
            if (text == ">") {
                return TokenType::kGreaterThanSign;
            } else if (text == ">=") {
                return TokenType::kGreaterThanEqualsSign;
            } else {
                return TokenType::kError;
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
                return TokenType::kMinusSign;
            } else if (text == ".") {
                return TokenType::kDot;
            } else if (std::regex_match(text, _integerRegex)) {
                return TokenType::kIntegerLiteral;
            } else if (std::regex_match(text, _decimalRegex)) {
                return TokenType::kDecimalLiteral;
            } else {
                return TokenType::kError;
            }
    }

    if (std::regex_match(text, _identifierRegex)) {
        auto lc = boost::algorithm::to_lower_copy(text);
        switch (lc[0]) {
            case 'a':
                if (lc == "and") {
                    return TokenType::kAnd;
                } else if (lc == "as") {
                    return TokenType::kAs;
                }
                break;
            case 'b':
                if (lc == "boolean") {
                    return TokenType::kBoolean;
                } else if (lc == "by") {
                    return TokenType::kBy;
                }
                break;
            case 'c':
                if (lc == "case") {
                    return TokenType::kCase;
                } else if (lc == "catch") {
                    return TokenType::kCatch;
                } else if (lc == "const") {
                    return TokenType::kConst;
                } else if (lc == "continue") {
                    return TokenType::kContinue;
                }
                break;
            case 'd':
                if (lc == "date") {
                    return TokenType::kDate;
                } else if (lc == "datetime") {
                    return TokenType::kDateTime;
                } else if (lc == "datetimeoffset") {
                    return TokenType::kDateTimeOffset;
                } else if (lc == "dim") {
                    return TokenType::kDim;
                } else if (lc == "do") {
                    return TokenType::kDo;
                }
                break;
            case 'e':
                if (lc == "else") {
                    return TokenType::kElse;
                } else if (lc == "end") {
                    return TokenType::kEnd;
                } else if (lc == "exit") {
                    return TokenType::kExit;
                }
                break;
            case 'f':
                if (lc == "false") {
                    return TokenType::kFalse;
                } else if (lc == "finally") {
                    return TokenType::kFinally;
                } else if (lc == "for") {
                    return TokenType::kFor;
                } else if (lc == "from") {
                    return TokenType::kFrom;
                } else if (lc == "function") {
                    return TokenType::kFunction;
                }
                break;
            case 'g':
                if (lc == "group") {
                    return TokenType::kGroup;
                }
                break;
            case 'i':
                if (lc == "if") {
                    return TokenType::kIf;
                } else if (lc == "in") {
                    return TokenType::kIn;
                } else if (lc == "integer") {
                    return TokenType::kInteger;
                } else if (lc == "into") {
                    return TokenType::kInto;
                }
                break;
            case 'j':
                if (lc == "join") {
                    return TokenType::kJoin;
                }
                break;
            case 'k':
                if (lc == "key") {
                    return TokenType::kKey;
                }
                break;
            case 'l':
                if (lc == "let") {
                    return TokenType::kLet;
                } else if (lc == "list") {
                    return TokenType::kList;
                } else if (lc == "loop") {
                    return TokenType::kLoop;
                }
                break;
            case 'm':
                if (lc == "map") {
                    return TokenType::kMap;
                } else if (lc == "mod") {
                    return TokenType::kMod;
                }
                break;
            case 'n':
                if (lc == "next") {
                    return TokenType::kNext;
                } else if (lc == "not") {
                    return TokenType::kNot;
                } else if (lc == "number") {
                    return TokenType::kNumber;
                }
                break;
            case 'o':
                if (lc == "of") {
                    return TokenType::kOf;
                } else if (lc == "on") {
                    return TokenType::kOn;
                } else if (lc == "optional") {
                    return TokenType::kOptional;
                } else if (lc == "or") {
                    return TokenType::kOr;
                }
                break;
            case 'q':
                if (lc == "query") {
                    return TokenType::kQuery;
                }
                break;
            case 'r':
                if (lc == "record") {
                    return TokenType::kRecord;
                } else if (lc == "rethrow") {
                    return TokenType::kRethrow;
                } else if (lc == "return") {
                    return TokenType::kReturn;
                }
                break;
            case 's':
                if (lc == "select") {
                    return TokenType::kSelect;
                } else if (lc == "step") {
                    return TokenType::kStep;
                } else if (lc == "sub") {
                    return TokenType::kSub;
                }
                break;
            case 't':
                if (lc == "text") {
                    return TokenType::kText;
                } else if (lc == "then") {
                    return TokenType::kThen;
                } else if (lc == "throw") {
                    return TokenType::kThrow;
                } else if (lc == "timespan") {
                    return TokenType::kTimeSpan;
                } else if (lc == "to") {
                    return TokenType::kTo;
                } else if (lc == "true") {
                    return TokenType::kTrue;
                } else if (lc == "try") {
                    return TokenType::kTry;
                } else if (lc == "type") {
                    return TokenType::kType;
                }
                break;
            case 'u':
                if (lc == "until") {
                    return TokenType::kUntil;
                }
                break;
            case 'w':
                if (lc == "wend") {
                    return TokenType::kWend;
                } else if (lc == "where") {
                    return TokenType::kWhere;
                } else if (lc == "while") {
                    return TokenType::kWhile;
                } else if (lc == "with") {
                    return TokenType::kWith;
                }
                break;
        }
        return TokenType::kIdentifier;
    } else {
        return TokenType::kError;
    }
}
