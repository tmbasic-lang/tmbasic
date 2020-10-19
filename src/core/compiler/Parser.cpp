#include "Parser.h"
#include "core/util/decimal.h"
#include "core/util/cast.h"

using namespace basic;
using namespace compiler;
using namespace util;

namespace compiler {

const int kNumCaptures = 5;

enum class TermType { kAnd, kCapture, kCut, kNonTerminal, kOptional, kOr, kTerminal, kZeroOrMore };

class Production;

class Term {
   public:
    TermType type;
    std::vector<Term> subTerms;
    const Production* production;  // when type = kNonTerminal
    basic::TokenType tokenType;    // when type = kTerminal
    int captureId;                 // when type = kCapture
    Term(TermType type) : type(type) {}
};

enum class BoxType { kNode, kToken };

class Box {
   public:
    BoxType type;
    Box(BoxType type) : type(type) {}
    virtual ~Box() = 0;
    virtual size_t count() = 0;
    virtual void removeAt(size_t index) = 0;
};

typedef std::array<std::unique_ptr<Box>, kNumCaptures> CaptureArray;

Box::~Box() {}

class NodeBox : public Box {
   public:
    std::vector<std::unique_ptr<Node>> values;

    NodeBox() : Box(BoxType::kNode) {}
    size_t count() override { return values.size(); }
    void removeAt(size_t index) override { values.erase(values.begin() + index); }
    void append(NodeBox& other) {
        for (auto& node : other.values) {
            values.push_back(std::move(node));
        }
    }
};

class TokenBox : public Box {
   public:
    std::vector<Token> values;

    TokenBox() : Box(BoxType::kToken) {}
    size_t count() override { return values.size(); }
    void removeAt(size_t index) override { values.erase(values.begin() + index); }
    void append(TokenBox& other) {
        for (auto& node : other.values) {
            values.push_back(node);
        }
    }
};

class Production {
   public:
    std::vector<Term> terms;
    Production(std::initializer_list<Term> terms) : terms(terms) {}
    virtual std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const = 0;
};

//
// terms
//

static Term prod(const Production* production) {
    auto t = Term(TermType::kNonTerminal);
    t.production = production;
    return t;
}

static Term term(TokenType tokenType) {
    auto t = Term(TermType::kTerminal);
    t.tokenType = tokenType;
    return t;
}

static Term optional(std::initializer_list<Term> terms) {
    auto t = Term(TermType::kOptional);
    for (auto& subTerm : terms) {
        t.subTerms.push_back(subTerm);
    }
    return t;
}

static Term list(std::initializer_list<Term> terms) {
    auto t = Term(TermType::kAnd);
    for (auto& subTerm : terms) {
        t.subTerms.push_back(subTerm);
    }
    return t;
}

static Term oneOf(std::initializer_list<Term> terms) {
    auto t = Term(TermType::kOr);
    for (auto& subTerm : terms) {
        t.subTerms.push_back(subTerm);
    }
    return t;
}

static Term zeroOrMore(std::initializer_list<Term> terms) {
    auto t = Term(TermType::kZeroOrMore);
    for (auto& subTerm : terms) {
        t.subTerms.push_back(subTerm);
    }
    return t;
}

static Term capture(int captureId, Term&& subTerm) {
    auto t = Term(TermType::kTerminal);
    t.captureId = captureId;
    t.subTerms.push_back(subTerm);
    return t;
}

static Term cut() {
    return Term(TermType::kCut);
}

//
// captures
//

template <typename T>
static std::vector<std::unique_ptr<T>> captureNodeArray(std::unique_ptr<Box> box) {
    if (box->count() == 0) {
        return {};
    }
    assert(box->type == BoxType::kNode);
    std::vector<std::unique_ptr<T>> items;
    auto nodeBox = dynamic_cast_move<NodeBox>(std::move(box));
    for (auto& x : nodeBox->values) {
        items.push_back(dynamic_cast_move<T>(std::move(x)));
    }
    return items;
}

template <typename T>
static std::unique_ptr<T> captureSingleNode(std::unique_ptr<Box> box) {
    assert(box->type == BoxType::kNode);
    auto nodeBox = dynamic_cast_move<NodeBox>(std::move(box));
    return dynamic_cast_move<T>(std::move(nodeBox->values[0]));
}

static bool hasCapture(std::unique_ptr<Box>& box) {
    return box->count() > 0;
}

template <typename T>
static std::unique_ptr<T> captureSingleNodeOrNull(std::unique_ptr<Box> box) {
    if (hasCapture(box)) {
        return captureSingleNode<T>(std::move(box));
    } else {
        return {};
    }
}

static Token captureToken(std::unique_ptr<Box> box) {
    assert(box->type == BoxType::kToken);
    auto tokenBox = dynamic_cast_move<TokenBox>(std::move(box));
    return tokenBox->values[0];
}

static Token captureTokenNoMove(const Box* box) {
    assert(box->type == BoxType::kToken);
    auto tokenBox = dynamic_cast<const TokenBox*>(box);
    return tokenBox->values[0];
}

static std::string captureTokenText(std::unique_ptr<Box> box) {
    return captureToken(std::move(box)).text;
}

static TokenType captureTokenType(std::unique_ptr<Box> box) {
    return captureToken(std::move(box)).type;
}

static std::unique_ptr<NodeBox> nodeBox(Node* node) {
    auto box = std::make_unique<NodeBox>();
    box->values.push_back(std::unique_ptr<Node>(node));
    return box;
}

static std::unique_ptr<TokenBox> tokenBox(Token token) {
    auto box = std::make_unique<TokenBox>();
    box->values.push_back(token);
    return box;
}

//
// helpers
//

static BinaryOperator tokenTypeToBinaryOperator(TokenType tokenType) {
    switch (tokenType) {
        case TokenType::kMultiplicationSign:
            return BinaryOperator::kMultiply;
        case TokenType::kDivisionSign:
            return BinaryOperator::kDivide;
        case TokenType::kMod:
            return BinaryOperator::kModulus;
        case TokenType::kPlusSign:
            return BinaryOperator::kAdd;
        case TokenType::kMinusSign:
            return BinaryOperator::kSubtract;
        case TokenType::kLessThanSign:
            return BinaryOperator::kLessThan;
        case TokenType::kLessThanEqualsSign:
            return BinaryOperator::kLessThanEquals;
        case TokenType::kGreaterThanSign:
            return BinaryOperator::kGreaterThan;
        case TokenType::kGreaterThanEqualsSign:
            return BinaryOperator::kGreaterThanEquals;
        case TokenType::kEqualsSign:
            return BinaryOperator::kEquals;
        case TokenType::kNotEqualsSign:
            return BinaryOperator::kNotEquals;
        case TokenType::kAnd:
            return BinaryOperator::kAnd;
        case TokenType::kOr:
            return BinaryOperator::kOr;
        default:
            assert(false);
            return {};
    }
}

static std::unique_ptr<Box> parseBinaryExpressionSuffix(CaptureArray& captures, const Token& firstToken) {
    return nodeBox(new BinaryExpressionSuffixNode(
        tokenTypeToBinaryOperator(captureTokenType(std::move(captures[0]))),
        captureSingleNode<ExpressionNode>(std::move(captures[1])), firstToken));
}

static std::unique_ptr<Box> parseBinaryExpression(CaptureArray& captures, const Token& firstToken) {
    if (hasCapture(captures[1])) {
        auto token = captureTokenNoMove(captures[1].get());
        return nodeBox(new BinaryExpressionNode(
            captureSingleNode<ExpressionNode>(std::move(captures[0])),
            captureNodeArray<BinaryExpressionSuffixNode>(std::move(captures[1])), token));
    } else {
        return std::move(captures[0]);
    }
}

//
// productions
//

class BodyProduction : public Production {
   public:
    BodyProduction(const Production* statement)
        : Production({
              zeroOrMore({
                  capture(0, prod(statement)),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto statements = captureNodeArray<StatementNode>(std::move(captures[0]));
        auto body = new BodyNode(statements, firstToken);
        return nodeBox(body);
    }
};

class ArgumentListProduction : public Production {
   public:
    ArgumentListProduction(const Production* expression)
        : Production({
              optional({
                  capture(0, prod(expression)),
                  zeroOrMore({
                      term(TokenType::kComma),
                      capture(0, prod(expression)),
                  }),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class ParameterProduction : public Production {
   public:
    ParameterProduction(const Production* type)
        : Production({
              capture(0, term(TokenType::kIdentifier)),
              cut(),
              term(TokenType::kAs),
              capture(1, prod(type)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto type = captureSingleNode<TypeNode>(std::move(captures[1]));
        return nodeBox(new ParameterNode(captureTokenText(std::move(captures[0])), std::move(type), firstToken));
    }
};

class ParameterListProduction : public Production {
   public:
    ParameterListProduction(const Production* parameter)
        : Production({
              optional({
                  capture(0, prod(parameter)),
                  cut(),
                  zeroOrMore({
                      term(TokenType::kComma),
                      capture(0, prod(parameter)),
                  }),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class NamedTypeProduction : public Production {
   public:
    NamedTypeProduction()
        : Production({
              capture(0, term(TokenType::kIdentifier)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto type = new TypeNode(Kind::kRecord, firstToken);
        type->recordName = std::optional(captureTokenText(std::move(captures[0])));
        return nodeBox(type);
    }
};

class TypeWithParenthesesProduction : public Production {
   public:
    TypeWithParenthesesProduction(const Production* type)
        : Production({
              oneOf({
                  list({
                      term(TokenType::kLeftParenthesis),
                      cut(),
                      capture(0, prod(type)),
                      term(TokenType::kRightParenthesis),
                  }),
                  capture(0, prod(type)),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class OptionalTypeProduction : public Production {
   public:
    OptionalTypeProduction(const Production* typeWithParentheses)
        : Production({
              term(TokenType::kOptional),
              cut(),
              capture(0, prod(typeWithParentheses)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto type = new TypeNode(Kind::kOptional, firstToken);
        type->optionalValueType = std::move(captureSingleNode<TypeNode>(std::move(captures[0])));
        return nodeBox(type);
    }
};

class MapTypeProduction : public Production {
   public:
    MapTypeProduction(const Production* typeWithParentheses)
        : Production({
              term(TokenType::kMap),
              cut(),
              term(TokenType::kFrom),
              capture(0, prod(typeWithParentheses)),
              term(TokenType::kTo),
              capture(1, prod(typeWithParentheses)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto type = new TypeNode(Kind::kMap, firstToken);
        type->mapKeyType = std::move(captureSingleNode<TypeNode>(std::move(captures[0])));
        type->mapValueType = std::move(captureSingleNode<TypeNode>(std::move(captures[1])));
        return nodeBox(type);
    }
};

class ListTypeProduction : public Production {
   public:
    ListTypeProduction(const Production* typeWithParentheses)
        : Production({
              term(TokenType::kList),
              cut(),
              term(TokenType::kOf),
              capture(0, prod(typeWithParentheses)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto type = new TypeNode(Kind::kList, firstToken);
        type->listItemType = std::move(captureSingleNode<TypeNode>(std::move(captures[0])));
        return nodeBox(type);
    }
};

class RecordTypeProduction : public Production {
   public:
    RecordTypeProduction(const Production* parameterList)
        : Production({
              term(TokenType::kRecord),
              cut(),
              term(TokenType::kLeftParenthesis),
              capture(0, prod(parameterList)),
              term(TokenType::kRightParenthesis),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto type = new TypeNode(Kind::kRecord, firstToken);
        for (auto& parameter : captureNodeArray<ParameterNode>(std::move(captures[0]))) {
            auto field = new FieldNode(parameter->name, std::move(parameter->type), firstToken);
            type->fields.push_back(std::unique_ptr<FieldNode>(field));
        }
        return nodeBox(type);
    }
};

class PrimitiveTypeProduction : public Production {
   public:
    PrimitiveTypeProduction()
        : Production({
              capture(
                  0,
                  oneOf({
                      term(TokenType::kBoolean),
                      term(TokenType::kNumber),
                      term(TokenType::kText),
                      term(TokenType::kDate),
                      term(TokenType::kDateTime),
                      term(TokenType::kDateTimeOffset),
                      term(TokenType::kTimeSpan),
                  })),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        Kind k;
        switch (captureTokenType(std::move(captures[0]))) {
            case TokenType::kBoolean:
                k = Kind::kBoolean;
                break;
            case TokenType::kNumber:
                k = Kind::kNumber;
                break;
            case TokenType::kText:
                k = Kind::kText;
                break;
            case TokenType::kDate:
                k = Kind::kDate;
                break;
            case TokenType::kDateTime:
                k = Kind::kDateTime;
                break;
            case TokenType::kDateTimeOffset:
                k = Kind::kDateTimeOffset;
                break;
            case TokenType::kTimeSpan:
                k = Kind::kTimeSpan;
                break;
            default:
                assert(false);
                k = Kind::kBoolean;
                break;
        }
        auto type = new TypeNode(k, firstToken);
        return nodeBox(type);
    }
};

class TypeProduction : public Production {
   public:
    TypeProduction() : Production({}) {}

    void init(
        const Production* primitiveType,
        const Production* recordType,
        const Production* listType,
        const Production* mapType,
        const Production* optionalType,
        const Production* namedType) {
        terms = {
            capture(
                0,
                oneOf({
                    prod(primitiveType),
                    prod(recordType),
                    prod(listType),
                    prod(mapType),
                    prod(optionalType),
                    prod(namedType),
                })),
        };
    }

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class LiteralValueProduction : public Production {
   public:
    LiteralValueProduction()
        : Production({
              capture(
                  0,
                  oneOf({
                      term(TokenType::kBooleanLiteral),
                      term(TokenType::kNumberLiteral),
                      term(TokenType::kStringLiteral),
                  })),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto token = captureToken(std::move(captures[0]));
        switch (token.type) {
            case TokenType::kBooleanLiteral: {
                auto lowercase = boost::algorithm::to_lower_copy(token.text);
                if (lowercase == "true") {
                    return nodeBox(new LiteralBooleanExpressionNode(true, firstToken));
                } else if (lowercase == "false") {
                    return nodeBox(new LiteralBooleanExpressionNode(false, firstToken));
                } else {
                    assert(false);
                    return {};
                }
                break;
            }

            case TokenType::kNumberLiteral:
                return nodeBox(new LiteralNumberExpressionNode(parseDecimalString(token.text), firstToken));

            case TokenType::kStringLiteral: {
                auto insidePart = token.text.substr(1, token.text.length() - 2);
                auto doubleQuote = std::string("\"\"");
                auto singleQuote = std::string("\"");
                auto pos = insidePart.find(doubleQuote);
                while (pos != std::string::npos) {
                    insidePart.replace(pos, doubleQuote.size(), singleQuote);
                    pos = insidePart.find(doubleQuote, pos + singleQuote.length());
                }
                return nodeBox(new LiteralStringExpressionNode(insidePart, firstToken));
            }

            default:
                assert(false);
                return {};
        }
    }
};

class LiteralRecordFieldProduction : public Production {
   public:
    LiteralRecordFieldProduction(const Production* expression)
        : Production({
              capture(0, term(TokenType::kIdentifier)),
              cut(),
              term(TokenType::kColon),
              capture(1, prod(expression)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new LiteralRecordFieldNode(
            captureTokenText(std::move(captures[0])), captureSingleNode<ExpressionNode>(std::move(captures[1])),
            firstToken));
    }
};

class LiteralRecordFieldListProduction : public Production {
   public:
    LiteralRecordFieldListProduction(const Production* literalRecordField)
        : Production({
              optional({
                  capture(0, prod(literalRecordField)),
                  cut(),
                  zeroOrMore({
                      term(TokenType::kComma),
                      capture(0, prod(literalRecordField)),
                  }),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class LiteralRecordTermProduction : public Production {
   public:
    LiteralRecordTermProduction(const Production* literalRecordFieldList)
        : Production({
              term(TokenType::kLeftBrace),
              cut(),
              capture(0, prod(literalRecordFieldList)),
              term(TokenType::kRightBrace),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new LiteralRecordExpressionNode(
            captureNodeArray<LiteralRecordFieldNode>(std::move(captures[0])), firstToken));
    }
};

class LiteralArrayTermProduction : public Production {
   public:
    LiteralArrayTermProduction(const Production* argumentList)
        : Production({
              term(TokenType::kLeftBracket),
              cut(),
              capture(0, prod(argumentList)),
              term(TokenType::kRightBracket),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(
            new LiteralArrayExpressionNode(captureNodeArray<ExpressionNode>(std::move(captures[0])), firstToken));
    }
};

class FunctionCallTermProduction : public Production {
   public:
    FunctionCallTermProduction(const Production* argumentList)
        : Production({
              capture(0, term(TokenType::kIdentifier)),
              term(TokenType::kLeftParenthesis),
              cut(),
              capture(1, prod(argumentList)),
              term(TokenType::kRightParenthesis),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new CallExpressionNode(
            captureTokenText(std::move(captures[0])), captureNodeArray<ExpressionNode>(std::move(captures[1])),
            firstToken));
    }
};

class ParenthesesTermProduction : public Production {
   public:
    ParenthesesTermProduction(const Production* expression)
        : Production({
              term(TokenType::kLeftParenthesis),
              cut(),
              capture(0, prod(expression)),
              term(TokenType::kRightParenthesis),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(
            new ParenthesesExpressionNode(captureSingleNode<ExpressionNode>(std::move(captures[0])), firstToken));
    }
};

class ExpressionTermProduction : public Production {
   public:
    ExpressionTermProduction(
        const Production* literalValue,
        const Production* parenthesesTerm,
        const Production* functionCallTerm,
        const Production* literalArrayTerm,
        const Production* literalRecordTerm)
        : Production({
              oneOf({
                  capture(0, prod(literalValue)),
                  capture(0, prod(parenthesesTerm)),
                  capture(0, prod(functionCallTerm)),
                  capture(0, prod(literalArrayTerm)),
                  capture(0, prod(literalRecordTerm)),
                  capture(1, term(TokenType::kIdentifier)),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        if (hasCapture(captures[0])) {
            return std::move(captures[0]);
        } else if (hasCapture(captures[1])) {
            return nodeBox(new SymbolReferenceExpressionNode(captureTokenText(std::move(captures[2])), firstToken));
        } else {
            assert(false);
            return {};
        }
    }
};

class DottedExpressionSuffixProduction : public Production {
   public:
    DottedExpressionSuffixProduction(const Production* argumentList)
        : Production({
              term(TokenType::kDot),
              cut(),
              capture(0, term(TokenType::kIdentifier)),
              optional({
                  term(TokenType::kLeftParenthesis),
                  capture(1, prod(argumentList)),
                  term(TokenType::kRightParenthesis),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto isCall = hasCapture(captures[1]);
        return nodeBox(new DottedExpressionSuffixNode(
            captureTokenText(std::move(captures[0])), isCall, captureNodeArray<ExpressionNode>(std::move(captures[1])),
            firstToken));
    }
};

class DottedExpressionProduction : public Production {
   public:
    DottedExpressionProduction(const Production* expressionTerm, const Production* dottedExpressionSuffix)
        : Production({
              capture(0, prod(expressionTerm)),
              zeroOrMore({
                  capture(1, prod(dottedExpressionSuffix)),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto dottedSuffixes = captureNodeArray<DottedExpressionSuffixNode>(std::move(captures[1]));
        if (dottedSuffixes.size() == 0) {
            return std::move(captures[0]);
        } else {
            auto token = dottedSuffixes[0]->token;
            return nodeBox(new DottedExpressionNode(
                captureSingleNode<ExpressionNode>(std::move(captures[0])), std::move(dottedSuffixes), token));
        }
    }
};

class ConvertExpressionProduction : public Production {
   public:
    ConvertExpressionProduction(const Production* dottedExpression, const Production* type)
        : Production({
              capture(0, prod(dottedExpression)),
              optional({
                  term(TokenType::kAs),
                  capture(1, prod(type)),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        if (hasCapture(captures[1])) {
            auto token = captureTokenNoMove(captures[1].get());
            return nodeBox(new ConvertExpressionNode(
                captureSingleNode<ExpressionNode>(std::move(captures[0])),
                captureSingleNode<TypeNode>(std::move(captures[1])), token));
        } else {
            return std::move(captures[0]);
        }
    }
};

class UnaryExpressionProduction : public Production {
   public:
    UnaryExpressionProduction(const Production* convertExpression)
        : Production({
              optional({
                  capture(0, term(TokenType::kNot)),
              }),
              capture(1, prod(convertExpression)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        if (hasCapture(captures[0])) {
            return nodeBox(
                new NotExpressionNode(captureSingleNode<ExpressionNode>(std::move(captures[1])), firstToken));
        } else {
            return std::move(captures[1]);
        }
    }
};

class MultiplyExpressionSuffixProduction : public Production {
   public:
    MultiplyExpressionSuffixProduction(const Production* unaryExpression)
        : Production({
              capture(
                  0,
                  oneOf({
                      term(TokenType::kMultiplicationSign),
                      term(TokenType::kDivisionSign),
                      term(TokenType::kMod),
                  })),
              capture(1, prod(unaryExpression)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return parseBinaryExpressionSuffix(captures, firstToken);
    }
};

class MultiplyExpressionProduction : public Production {
   public:
    MultiplyExpressionProduction(const Production* unaryExpression, const Production* multiplyExpressionSuffix)
        : Production({
              capture(0, prod(unaryExpression)),
              zeroOrMore({
                  capture(1, prod(multiplyExpressionSuffix)),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return parseBinaryExpression(captures, firstToken);
    }
};

class AddExpressionSuffixProduction : public Production {
   public:
    AddExpressionSuffixProduction(const Production* multiplyExpression)
        : Production({
              capture(
                  0,
                  oneOf({
                      term(TokenType::kPlusSign),
                      term(TokenType::kMinusSign),
                  })),
              capture(1, prod(multiplyExpression)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return parseBinaryExpressionSuffix(captures, firstToken);
    }
};

class AddExpressionProduction : public Production {
   public:
    AddExpressionProduction(const Production* multiplyExpression, const Production* addExpressionSuffix)
        : Production({
              capture(0, prod(multiplyExpression)),
              zeroOrMore({
                  capture(1, prod(addExpressionSuffix)),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return parseBinaryExpression(captures, firstToken);
    }
};

class InequalityExpressionSuffixProduction : public Production {
   public:
    InequalityExpressionSuffixProduction(const Production* addExpression)
        : Production({
              capture(
                  0,
                  oneOf({
                      term(TokenType::kLessThanSign),
                      term(TokenType::kLessThanEqualsSign),
                      term(TokenType::kGreaterThanSign),
                      term(TokenType::kGreaterThanEqualsSign),
                  })),
              capture(1, prod(addExpression)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return parseBinaryExpressionSuffix(captures, firstToken);
    }
};

class InequalityExpressionProduction : public Production {
   public:
    InequalityExpressionProduction(const Production* addExpression, const Production* inequalityExpressionSuffix)
        : Production({
              capture(0, prod(addExpression)),
              zeroOrMore({
                  capture(1, prod(inequalityExpressionSuffix)),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return parseBinaryExpression(captures, firstToken);
    }
};

class EqualityExpressionSuffixProduction : public Production {
   public:
    EqualityExpressionSuffixProduction(const Production* inequalityExpression)
        : Production({
              capture(
                  0,
                  oneOf({
                      term(TokenType::kEqualsSign),
                      term(TokenType::kNotEqualsSign),
                  })),
              capture(1, prod(inequalityExpression)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return parseBinaryExpressionSuffix(captures, firstToken);
    }
};

class EqualityExpressionProduction : public Production {
   public:
    EqualityExpressionProduction(const Production* inequalityExpression, const Production* equalityExpressionSuffix)
        : Production({
              capture(0, prod(inequalityExpression)),
              zeroOrMore({
                  capture(1, prod(equalityExpressionSuffix)),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return parseBinaryExpression(captures, firstToken);
    }
};

class AndExpressionSuffixProduction : public Production {
   public:
    AndExpressionSuffixProduction(const Production* equalityExpression)
        : Production({
              capture(0, term(TokenType::kAnd)),
              capture(1, prod(equalityExpression)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return parseBinaryExpressionSuffix(captures, firstToken);
    }
};

class AndExpressionProduction : public Production {
   public:
    AndExpressionProduction(const Production* equalityExpression, const Production* andExpressionSuffix)
        : Production({
              capture(0, prod(equalityExpression)),
              zeroOrMore({
                  capture(1, prod(andExpressionSuffix)),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return parseBinaryExpression(captures, firstToken);
    }
};

class OrExpressionSuffixProduction : public Production {
   public:
    OrExpressionSuffixProduction(const Production* andExpression)
        : Production({
              capture(0, term(TokenType::kOr)),
              capture(1, prod(andExpression)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return parseBinaryExpressionSuffix(captures, firstToken);
    }
};

class OrExpressionProduction : public Production {
   public:
    OrExpressionProduction(const Production* andExpression, const Production* orExpressionSuffix)
        : Production({
              capture(0, prod(andExpression)),
              zeroOrMore({
                  capture(1, prod(orExpressionSuffix)),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return parseBinaryExpression(captures, firstToken);
    }
};

class ExpressionProduction : public Production {
   public:
    ExpressionProduction() : Production({}) {}

    void init(const Production* orExpression) { terms = { capture(0, prod(orExpression)) }; }

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class FinallyBlockProduction : public Production {
   public:
    FinallyBlockProduction(const Production* body)
        : Production({
              term(TokenType::kFinally),
              cut(),
              term(TokenType::kEndOfLine),
              capture(0, prod(body)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class CatchBlockProduction : public Production {
   public:
    CatchBlockProduction(const Production* body)
        : Production({
              term(TokenType::kCatch),
              cut(),
              term(TokenType::kEndOfLine),
              capture(0, prod(body)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class TryStatementProduction : public Production {
   public:
    TryStatementProduction(const Production* body, const Production* catchBlock, const Production* finallyBlock)
        : Production({
              term(TokenType::kTry),
              cut(),
              term(TokenType::kEndOfLine),
              capture(0, prod(body)),
              optional({
                  capture(1, prod(catchBlock)),
              }),
              optional({
                  capture(2, prod(finallyBlock)),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new TryStatementNode(
            captureSingleNode<BodyNode>(std::move(captures[0])),
            captureSingleNodeOrNull<BodyNode>(std::move(captures[1])),
            captureSingleNodeOrNull<BodyNode>(std::move(captures[2])), firstToken));
    }
};

class RethrowStatementProduction : public Production {
   public:
    RethrowStatementProduction()
        : Production({
              term(TokenType::kRethrow),
              cut(),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new RethrowStatementNode(firstToken));
    }
};

class ThrowStatementProduction : public Production {
   public:
    ThrowStatementProduction(const Production* expression)
        : Production({
              term(TokenType::kThrow),
              cut(),
              capture(0, prod(expression)),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new ThrowStatementNode(captureSingleNode<ExpressionNode>(std::move(captures[0])), firstToken));
    }
};

class ExitStatementProduction : public Production {
   public:
    ExitStatementProduction()
        : Production({
              term(TokenType::kExit),
              capture(
                  0,
                  oneOf({
                      term(TokenType::kDo),
                      term(TokenType::kFor),
                      term(TokenType::kSelect),
                      term(TokenType::kTry),
                      term(TokenType::kWhile),
                  })),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        ExitScope scope;
        switch (captureTokenType(std::move(captures[0]))) {
            case TokenType::kDo:
                scope = ExitScope::kDo;
                break;
            case TokenType::kFor:
                scope = ExitScope::kFor;
                break;
            case TokenType::kSelect:
                scope = ExitScope::kSelectCase;
                break;
            case TokenType::kTry:
                scope = ExitScope::kTry;
                break;
            case TokenType::kWhile:
                scope = ExitScope::kWhile;
                break;
            default:
                assert(false);
                scope = {};
                break;
        }
        return nodeBox(new ExitStatementNode(scope, firstToken));
    }
};

class ContinueStatementProduction : public Production {
   public:
    ContinueStatementProduction()
        : Production({
              term(TokenType::kExit),
              capture(
                  0,
                  oneOf({
                      term(TokenType::kDo),
                      term(TokenType::kFor),
                      term(TokenType::kWhile),
                  })),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        ContinueScope scope;
        switch (captureTokenType(std::move(captures[0]))) {
            case TokenType::kDo:
                scope = ContinueScope::kDo;
                break;
            case TokenType::kFor:
                scope = ContinueScope::kFor;
                break;
            case TokenType::kWhile:
                scope = ContinueScope::kWhile;
                break;
            default:
                assert(false);
                scope = {};
                break;
        }
        return nodeBox(new ContinueStatementNode(scope, firstToken));
    }
};

class CallStatementProduction : public Production {
   public:
    CallStatementProduction(const Production* argumentList)
        : Production({
              capture(0, term(TokenType::kIdentifier)),
              cut(),
              capture(1, prod(argumentList)),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new CallStatementNode(
            captureTokenText(std::move(captures[0])), captureNodeArray<ExpressionNode>(std::move(captures[1])),
            firstToken));
    }
};

class ReturnStatementProduction : public Production {
   public:
    ReturnStatementProduction(const Production* expression)
        : Production({
              term(TokenType::kReturn),
              cut(),
              optional({
                  capture(0, prod(expression)),
              }),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(
            new ReturnStatementNode(captureSingleNodeOrNull<ExpressionNode>(std::move(captures[0])), firstToken));
    }
};

class SelectStatementProduction : public Production {
   public:
    SelectStatementProduction(const Production* expression)
        : Production({
              term(TokenType::kSelect),
              // no cut here, since it could be 'select' 'case'
              capture(0, prod(expression)),
              cut(),  // instead the cut is here
              optional({
                  term(TokenType::kTo),
                  capture(1, prod(expression)),
              }),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new SelectStatementNode(
            captureSingleNode<ExpressionNode>(std::move(captures[0])),
            captureSingleNodeOrNull<ExpressionNode>(std::move(captures[1])), firstToken));
    }
};

class ConstStatementProduction : public Production {
   public:
    ConstStatementProduction(const Production* literalValue)
        : Production({
              term(TokenType::kConst),
              cut(),
              capture(0, term(TokenType::kIdentifier)),
              term(TokenType::kEqualsSign),
              capture(1, prod(literalValue)),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto token = captureTokenNoMove(captures[0].get());
        return nodeBox(new ConstStatementNode(
            captureTokenText(std::move(captures[0])),
            captureSingleNode<ConstValueExpressionNode>(std::move(captures[1])), token));
    }
};

class AssignLocationSuffixProduction : public Production {
   public:
    AssignLocationSuffixProduction(const Production* expression)
        : Production({
              oneOf({
                  list({
                      term(TokenType::kDot),
                      capture(0, term(TokenType::kIdentifier)),
                  }),
                  list({
                      term(TokenType::kLeftParenthesis),
                      capture(1, prod(expression)),
                      term(TokenType::kRightParenthesis),
                  }),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        if (hasCapture(captures[0])) {
            return nodeBox(new AssignLocationSuffixNode(captureTokenText(std::move(captures[0])), firstToken));
        } else {
            return nodeBox(
                new AssignLocationSuffixNode(captureSingleNode<ExpressionNode>(std::move(captures[1])), firstToken));
        }
    }
};

class AssignStatementProduction : public Production {
   public:
    AssignStatementProduction(const Production* assignLocationSuffix, const Production* expression)
        : Production({
              capture(0, term(TokenType::kIdentifier)),
              zeroOrMore({
                  capture(1, prod(assignLocationSuffix)),
              }),
              term(TokenType::kEqualsSign),
              capture(2, prod(expression)),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new AssignStatementNode(
            captureTokenText(std::move(captures[0])),
            captureNodeArray<AssignLocationSuffixNode>(std::move(captures[1])),
            captureSingleNode<ExpressionNode>(std::move(captures[2])), firstToken));
    }
};

class DimStatementProduction : public Production {
   public:
    DimStatementProduction(const Production* type, const Production* expression)
        : Production({
              term(TokenType::kDim),  // no cut here because it could be "dim list"
              capture(0, term(TokenType::kIdentifier)),
              cut(),  // cut here instead
              oneOf({
                  list({
                      term(TokenType::kAs),
                      capture(1, prod(type)),
                  }),
                  list({
                      term(TokenType::kEqualsSign),
                      capture(2, prod(expression)),
                  }),
              }),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        if (hasCapture(captures[1])) {
            return nodeBox(new DimStatementNode(
                captureTokenText(std::move(captures[0])), captureSingleNode<TypeNode>(std::move(captures[1])),
                firstToken));
        } else {
            return nodeBox(new DimStatementNode(
                captureTokenText(std::move(captures[0])), captureSingleNode<ExpressionNode>(std::move(captures[2])),
                firstToken));
        }
    }
};

class DimCollectionStatementProduction : public Production {
   public:
    DimCollectionStatementProduction(const Production* body)
        : Production({
              term(TokenType::kDim),
              capture(
                  0,
                  oneOf({
                      term(TokenType::kList),
                      term(TokenType::kMap),
                  })),
              capture(1, term(TokenType::kIdentifier)),
              term(TokenType::kEndOfLine),
              capture(2, prod(body)),
              term(TokenType::kEnd),
              term(TokenType::kDim),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        CollectionType type;
        switch (captureTokenType(std::move(captures[0]))) {
            case TokenType::kList:
                type = CollectionType::kList;
                break;
            case TokenType::kMap:
                type = CollectionType::kMap;
                break;
            default:
                assert(false);
                type = {};
                break;
        }
        return nodeBox(new DimCollectionStatementNode(
            captureTokenText(std::move(captures[1])), type, captureSingleNode<BodyNode>(std::move(captures[2])),
            firstToken));
    }
};

class CaseValueProduction : public Production {
   public:
    CaseValueProduction(const Production* expression)
        : Production({
              capture(0, prod(expression)),
              optional({
                  term(TokenType::kTo),
                  capture(1, prod(expression)),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new CaseValueNode(
            captureSingleNode<ExpressionNode>(std::move(captures[0])),
            captureSingleNodeOrNull<ExpressionNode>(std::move(captures[1])), firstToken));
    }
};

class CaseValueListProduction : public Production {
   public:
    CaseValueListProduction(const Production* caseValue)
        : Production({
              capture(0, prod(caseValue)),
              zeroOrMore({
                  term(TokenType::kComma),
                  capture(0, prod(caseValue)),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class CaseProduction : public Production {
   public:
    CaseProduction(const Production* caseValueList, const Production* body)
        : Production({
              term(TokenType::kCase),
              cut(),
              oneOf({
                  capture(0, prod(caseValueList)),
                  term(TokenType::kElse),
              }),
              term(TokenType::kEndOfLine),
              capture(1, prod(body)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new CaseNode(
            captureNodeArray<CaseValueNode>(std::move(captures[0])),
            captureSingleNode<BodyNode>(std::move(captures[1])), firstToken));
    }
};

class SelectCaseStatementProduction : public Production {
   public:
    SelectCaseStatementProduction(const Production* expression, const Production* case_)
        : Production({
              term(TokenType::kSelect),
              term(TokenType::kCase),
              cut(),
              capture(0, prod(expression)),
              term(TokenType::kEndOfLine),
              zeroOrMore({
                  capture(1, prod(case_)),
              }),
              term(TokenType::kEnd),
              term(TokenType::kSelect),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new SelectCaseStatementNode(
            captureSingleNode<ExpressionNode>(std::move(captures[0])),
            captureNodeArray<CaseNode>(std::move(captures[1])), firstToken));
    }
};

class GroupStatementProduction : public Production {
   public:
    GroupStatementProduction(const Production* expression, const Production* body)
        : Production({
              term(TokenType::kGroup),
              cut(),
              capture(0, prod(expression)),
              term(TokenType::kBy),
              capture(1, prod(expression)),
              term(TokenType::kInto),
              capture(2, term(TokenType::kIdentifier)),
              optional({
                  term(TokenType::kWith),
                  term(TokenType::kKey),
                  capture(3, term(TokenType::kIdentifier)),
              }),
              term(TokenType::kEndOfLine),
              capture(4, prod(body)),
              term(TokenType::kNext),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto groupKeyNameToken = captureTokenNoMove(captures[3].get());
        return nodeBox(new GroupStatementNode(
            captureSingleNode<ExpressionNode>(std::move(captures[0])),
            captureSingleNode<ExpressionNode>(std::move(captures[1])), captureTokenText(std::move(captures[2])),
            std::make_unique<GroupKeyNameNode>(captureTokenText(std::move(captures[3])), groupKeyNameToken),
            captureSingleNode<BodyNode>(std::move(captures[4])), firstToken));
    }
};

class JoinStatementProduction : public Production {
   public:
    JoinStatementProduction(const Production* expression, const Production* body)
        : Production({
              term(TokenType::kJoin),
              cut(),
              capture(0, term(TokenType::kIdentifier)),
              term(TokenType::kIn),
              capture(1, prod(expression)),
              term(TokenType::kOn),
              capture(2, prod(expression)),
              term(TokenType::kEndOfLine),
              capture(1, prod(body)),
              term(TokenType::kNext),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new JoinStatementNode(
            captureTokenText(std::move(captures[0])), captureSingleNode<ExpressionNode>(std::move(captures[1])),
            captureSingleNode<ExpressionNode>(std::move(captures[2])),
            captureSingleNode<BodyNode>(std::move(captures[3])), firstToken));
    }
};

class DoConditionProduction : public Production {
   public:
    DoConditionProduction(const Production* expression)
        : Production({
              capture(
                  0,
                  oneOf({
                      term(TokenType::kWhile),
                      term(TokenType::kUntil),
                  })),
              cut(),
              capture(1, prod(expression)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        DoConditionType type;
        switch (captureTokenType(std::move(captures[0]))) {
            case TokenType::kWhile:
                type = DoConditionType::kWhile;
                break;
            case TokenType::kUntil:
                type = DoConditionType::kUntil;
                break;
            default:
                assert(false);
                type = {};
                break;
        }
        return nodeBox(
            new DoConditionNode(captureSingleNode<ExpressionNode>(std::move(captures[1])), type, firstToken));
    }
};

class DoStatementProduction : public Production {
   public:
    DoStatementProduction(const Production* doCondition, const Production* body)
        : Production({
              term(TokenType::kDo),
              cut(),
              oneOf({
                  list({
                      capture(0, prod(doCondition)),
                      term(TokenType::kEndOfLine),
                      capture(1, prod(body)),
                      term(TokenType::kLoop),
                  }),
                  list({
                      term(TokenType::kEndOfLine),
                      capture(1, prod(body)),
                      term(TokenType::kLoop),
                      capture(2, prod(doCondition)),
                  }),
              }),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto isBefore = hasCapture(captures[0]);
        auto position = isBefore ? DoConditionPosition::kBeforeBody : DoConditionPosition::kAfterBody;
        return nodeBox(new DoStatementNode(
            captureSingleNode<DoConditionNode>(std::move(captures[isBefore ? 0 : 2])), position,
            captureSingleNode<BodyNode>(std::move(captures[1])), firstToken));
    }
};

class ElseIfProduction : public Production {
   public:
    ElseIfProduction(const Production* expression, const Production* body)
        : Production({
              term(TokenType::kElse),
              term(TokenType::kIf),
              capture(0, prod(expression)),
              term(TokenType::kThen),
              term(TokenType::kEndOfLine),
              capture(1, prod(body)),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new ElseIfNode(
            captureSingleNode<ExpressionNode>(std::move(captures[0])),
            captureSingleNode<BodyNode>(std::move(captures[1])), firstToken));
    }
};

class IfStatementProduction : public Production {
   public:
    IfStatementProduction(const Production* expression, const Production* body, const Production* elseIf)
        : Production({
              term(TokenType::kIf),
              cut(),
              capture(0, prod(expression)),
              term(TokenType::kThen),
              term(TokenType::kEndOfLine),
              capture(1, prod(body)),
              zeroOrMore({
                  capture(2, prod(elseIf)),
              }),
              optional({
                  term(TokenType::kElse),
                  term(TokenType::kEndOfLine),
                  capture(3, prod(body)),
              }),
              term(TokenType::kEnd),
              term(TokenType::kIf),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new IfStatementNode(
            captureSingleNode<ExpressionNode>(std::move(captures[0])),
            captureSingleNode<BodyNode>(std::move(captures[1])), captureNodeArray<ElseIfNode>(std::move(captures[2])),
            captureSingleNodeOrNull<BodyNode>(std::move(captures[3])), firstToken));
    }
};

class WhileStatementProduction : public Production {
   public:
    WhileStatementProduction(const Production* expression, const Production* body)
        : Production({
              term(TokenType::kWhile),
              cut(),
              capture(0, prod(expression)),
              term(TokenType::kEndOfLine),
              capture(1, prod(body)),
              term(TokenType::kWend),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new WhileStatementNode(
            captureSingleNode<ExpressionNode>(std::move(captures[0])),
            captureSingleNode<BodyNode>(std::move(captures[1])), firstToken));
    }
};

class ForEachStatementProduction : public Production {
   public:
    ForEachStatementProduction(const Production* expression, const Production* body)
        : Production({
              term(TokenType::kFrom),
              cut(),
              capture(0, term(TokenType::kIdentifier)),
              term(TokenType::kIn),
              capture(1, prod(expression)),
              term(TokenType::kEndOfLine),
              capture(2, prod(body)),
              term(TokenType::kNext),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new ForEachStatementNode(
            captureTokenText(std::move(captures[0])), captureSingleNode<ExpressionNode>(std::move(captures[1])),
            captureSingleNode<BodyNode>(std::move(captures[2])), firstToken));
    }
};

class ForStepProduction : public Production {
   public:
    ForStepProduction()
        : Production({
              term(TokenType::kStep),
              cut(),
              capture(
                  0,
                  oneOf({
                      term(TokenType::kNumberLiteral),
                      term(TokenType::kIdentifier),
                  })),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto t = captureToken(std::move(captures[0]));
        switch (t.type) {
            case TokenType::kNumberLiteral:
                return nodeBox(new ForStepNode(parseDecimalString(t.text), firstToken));
            case TokenType::kIdentifier:
                return nodeBox(new ForStepNode(std::make_unique<SymbolReferenceExpressionNode>(t.text, t), firstToken));
            default:
                assert(false);
                return {};
        }
    }
};

class ForStatementProduction : public Production {
   public:
    ForStatementProduction(const Production* expression, const Production* forStep, const Production* body)
        : Production({
              term(TokenType::kFor),  // no cut here because it could be FOR EACH
              capture(0, term(TokenType::kIdentifier)),
              cut(),
              term(TokenType::kEqualsSign),
              capture(1, prod(expression)),
              term(TokenType::kTo),
              capture(2, prod(expression)),
              optional({
                  capture(3, prod(forStep)),
              }),
              term(TokenType::kEndOfLine),
              capture(4, prod(body)),
              term(TokenType::kNext),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new ForStatementNode(
            captureTokenText(std::move(captures[0])), captureSingleNode<ExpressionNode>(std::move(captures[1])),
            captureSingleNode<ExpressionNode>(std::move(captures[2])),
            captureSingleNodeOrNull<ForStepNode>(std::move(captures[3])),
            captureSingleNode<BodyNode>(std::move(captures[4])), firstToken));
    }
};

class CommandStatementProduction : public Production {
   public:
    CommandStatementProduction(
        const Production* assignStatement,
        const Production* selectStatement,
        const Production* returnStatement,
        const Production* callStatement,
        const Production* continueStatement,
        const Production* exitStatement,
        const Production* throwStatement,
        const Production* rethrowStatement)
        : Production({
              capture(
                  0,
                  oneOf({
                      prod(assignStatement),
                      prod(selectStatement),
                      prod(returnStatement),
                      prod(callStatement),
                      prod(continueStatement),
                      prod(exitStatement),
                      prod(throwStatement),
                      prod(rethrowStatement),
                  })),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class StatementProduction : public Production {
   public:
    StatementProduction() : Production({}) {}

    void init(
        const Production* commandStatement,
        const Production* forStatement,
        const Production* forEachStatement,
        const Production* whileStatement,
        const Production* doStatement,
        const Production* ifStatement,
        const Production* joinStatement,
        const Production* groupStatement,
        const Production* selectCaseStatement,
        const Production* tryStatement,
        const Production* dimStatement,
        const Production* dimCollectionStatement,
        const Production* constStatement) {
        terms = {
            oneOf({
                prod(commandStatement),
                prod(forStatement),
                prod(forEachStatement),
                prod(whileStatement),
                prod(doStatement),
                prod(ifStatement),
                prod(joinStatement),
                prod(groupStatement),
                prod(selectCaseStatement),
                prod(tryStatement),
                prod(dimStatement),
                prod(dimCollectionStatement),
                prod(constStatement),
            }),
        };
    }

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class TypeDeclarationProduction : public Production {
   public:
    TypeDeclarationProduction(const Production* parameter)
        : Production({
              term(TokenType::kType),
              cut(),
              capture(0, term(TokenType::kIdentifier)),
              term(TokenType::kEndOfLine),
              zeroOrMore({
                  capture(1, prod(parameter)),
                  term(TokenType::kEndOfLine),
              }),
              term(TokenType::kEnd),
              term(TokenType::kType),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new TypeDeclarationNode(
            captureTokenText(std::move(captures[0])), captureNodeArray<ParameterNode>(std::move(captures[1])),
            firstToken));
    }
};

class FunctionProduction : public Production {
   public:
    FunctionProduction(const Production* parameterList, const Production* type, const Production* body)
        : Production({
              term(TokenType::kFunction),
              cut(),
              capture(0, term(TokenType::kIdentifier)),
              term(TokenType::kLeftParenthesis),
              capture(1, prod(parameterList)),
              term(TokenType::kRightParenthesis),
              term(TokenType::kAs),
              capture(2, prod(type)),
              term(TokenType::kEndOfLine),
              capture(3, prod(body)),
              term(TokenType::kEnd),
              term(TokenType::kFunction),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new ProcedureNode(
            captureTokenText(std::move(captures[0])), captureNodeArray<ParameterNode>(std::move(captures[1])),
            captureSingleNode<TypeNode>(std::move(captures[2])), captureSingleNode<BodyNode>(std::move(captures[3])),
            firstToken));
    }
};

class SubroutineProduction : public Production {
   public:
    SubroutineProduction(const Production* parameterList, const Production* body)
        : Production({
              term(TokenType::kSub),
              cut(),
              capture(0, term(TokenType::kIdentifier)),
              term(TokenType::kLeftParenthesis),
              capture(1, prod(parameterList)),
              term(TokenType::kRightParenthesis),
              term(TokenType::kEndOfLine),
              capture(2, prod(body)),
              term(TokenType::kEnd),
              term(TokenType::kSub),
              term(TokenType::kEndOfLine),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new ProcedureNode(
            captureTokenText(std::move(captures[0])), captureNodeArray<ParameterNode>(std::move(captures[1])),
            captureSingleNode<BodyNode>(std::move(captures[2])), firstToken));
    }
};

class MemberProduction : public Production {
   public:
    MemberProduction(
        const Production* subroutine,
        const Production* function,
        const Production* dimStatement,
        const Production* constStatement,
        const Production* typeDeclaration)
        : Production({
              capture(
                  0,
                  oneOf({
                      prod(subroutine),
                      prod(function),
                      prod(dimStatement),
                      prod(constStatement),
                      prod(typeDeclaration),
                  })),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class ProgramProduction : public Production {
   public:
    ProgramProduction(const Production* member)
        : Production({
              zeroOrMore({
                  capture(0, prod(member)),
              }),
          }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class ProductionCollection {
   public:
    const Production* memberProduction;
    const Production* programProduction;

    ProductionCollection() {
        auto statement = dynamic_cast<StatementProduction*>(add(new StatementProduction()));
        auto expression = dynamic_cast<ExpressionProduction*>(add(new ExpressionProduction()));
        auto type = dynamic_cast<TypeProduction*>(add(new TypeProduction()));

        auto body = add(new BodyProduction(statement));
        auto argumentList = add(new ArgumentListProduction(expression));
        auto parameter = add(new ParameterProduction(type));
        auto parameterList = add(new ParameterListProduction(parameter));
        auto namedType = add(new NamedTypeProduction());
        auto typeWithParentheses = add(new TypeWithParenthesesProduction(type));
        auto optionalType = add(new OptionalTypeProduction(typeWithParentheses));
        auto mapType = add(new MapTypeProduction(typeWithParentheses));
        auto listType = add(new ListTypeProduction(typeWithParentheses));
        auto recordType = add(new RecordTypeProduction(parameterList));
        auto primitiveType = add(new PrimitiveTypeProduction());
        type->init(primitiveType, recordType, listType, mapType, optionalType, namedType);
        auto literalValue = add(new LiteralValueProduction());
        auto literalRecordField = add(new LiteralRecordFieldProduction(expression));
        auto literalRecordFieldList = add(new LiteralRecordFieldListProduction(literalRecordField));
        auto literalRecordTerm = add(new LiteralRecordTermProduction(literalRecordFieldList));
        auto literalArrayTerm = add(new LiteralArrayTermProduction(argumentList));
        auto functionCallTerm = add(new FunctionCallTermProduction(argumentList));
        auto parenthesesTerm = add(new ParenthesesTermProduction(expression));
        auto expressionTerm = add(new ExpressionTermProduction(
            literalValue, parenthesesTerm, functionCallTerm, literalArrayTerm, literalRecordTerm));
        auto dottedExpressionSuffix = add(new DottedExpressionSuffixProduction(argumentList));
        auto dottedExpression = add(new DottedExpressionProduction(expressionTerm, dottedExpressionSuffix));
        auto convertExpression = add(new ConvertExpressionProduction(dottedExpression, type));
        auto unaryExpression = add(new UnaryExpressionProduction(convertExpression));
        auto multiplyExpressionSuffix = add(new MultiplyExpressionSuffixProduction(unaryExpression));
        auto multiplyExpression = add(new MultiplyExpressionProduction(unaryExpression, multiplyExpressionSuffix));
        auto addExpressionSuffix = add(new AddExpressionSuffixProduction(multiplyExpression));
        auto addExpression = add(new AddExpressionProduction(multiplyExpression, addExpressionSuffix));
        auto inequalityExpressionSuffix = add(new InequalityExpressionSuffixProduction(addExpression));
        auto inequalityExpression = add(new InequalityExpressionProduction(addExpression, inequalityExpressionSuffix));
        auto equalityExpressionSuffix = add(new EqualityExpressionSuffixProduction(inequalityExpression));
        auto equalityExpression = add(new EqualityExpressionProduction(inequalityExpression, equalityExpressionSuffix));
        auto andExpressionSuffix = add(new AndExpressionSuffixProduction(equalityExpression));
        auto andExpression = add(new AndExpressionProduction(equalityExpression, andExpressionSuffix));
        auto orExpressionSuffix = add(new OrExpressionSuffixProduction(andExpression));
        auto orExpression = add(new OrExpressionProduction(andExpression, orExpressionSuffix));
        expression->init(orExpression);
        auto finallyBlock = add(new FinallyBlockProduction(body));
        auto catchBlock = add(new CatchBlockProduction(body));
        auto tryStatement = add(new TryStatementProduction(body, catchBlock, finallyBlock));
        auto rethrowStatement = add(new RethrowStatementProduction());
        auto throwStatement = add(new ThrowStatementProduction(expression));
        auto exitStatement = add(new ExitStatementProduction());
        auto continueStatement = add(new ContinueStatementProduction());
        auto callStatement = add(new CallStatementProduction(argumentList));
        auto returnStatement = add(new ReturnStatementProduction(expression));
        auto selectStatement = add(new SelectStatementProduction(expression));
        auto constStatement = add(new ConstStatementProduction(literalValue));
        auto assignLocationSuffix = add(new AssignLocationSuffixProduction(expression));
        auto assignStatement = add(new AssignStatementProduction(assignLocationSuffix, expression));
        auto dimStatement = add(new DimStatementProduction(type, expression));
        auto dimCollectionStatement = add(new DimCollectionStatementProduction(body));
        auto caseValue = add(new CaseValueProduction(expression));
        auto caseValueList = add(new CaseValueListProduction(caseValue));
        auto case_ = add(new CaseProduction(caseValueList, body));
        auto selectCaseStatement = add(new SelectCaseStatementProduction(expression, case_));
        auto groupStatement = add(new GroupStatementProduction(expression, body));
        auto joinStatement = add(new JoinStatementProduction(expression, body));
        auto doCondition = add(new DoConditionProduction(expression));
        auto doStatement = add(new DoStatementProduction(doCondition, body));
        auto elseIf = add(new ElseIfProduction(expression, body));
        auto ifStatement = add(new IfStatementProduction(expression, body, elseIf));
        auto whileStatement = add(new WhileStatementProduction(expression, body));
        auto forEachStatement = add(new ForEachStatementProduction(expression, body));
        auto forStep = add(new ForStepProduction());
        auto forStatement = add(new ForStatementProduction(expression, forStep, body));
        auto commandStatement = add(new CommandStatementProduction(
            assignStatement, selectStatement, returnStatement, callStatement, continueStatement, exitStatement,
            throwStatement, rethrowStatement));
        statement->init(
            commandStatement, forStatement, forEachStatement, whileStatement, doStatement, ifStatement, joinStatement,
            groupStatement, selectCaseStatement, tryStatement, dimStatement, dimCollectionStatement, constStatement);
        auto typeDeclaration = add(new TypeDeclarationProduction(parameter));
        auto function = add(new FunctionProduction(parameterList, type, body));
        auto subroutine = add(new SubroutineProduction(parameterList, body));
        auto member = add(new MemberProduction(subroutine, function, dimStatement, constStatement, typeDeclaration));
        auto program = add(new ProgramProduction(member));

        memberProduction = member;
        programProduction = program;
    }

   private:
    std::vector<std::unique_ptr<Production>> _productions;

    Production* add(Production* production) {
        _productions.push_back(std::unique_ptr<Production>(production));
        return production;
    }
};

class InputState {
   public:
    const std::vector<Token>& tokens;
    size_t tokenIndex;

    InputState(const std::vector<Token>& tokens)
        : tokens(tokens), tokenIndex(0), _endOfFileToken(Token(-1, 0, TokenType::kEndOfFile, "")) {}

    const Token& currentToken() {
        if (tokenIndex < tokens.size()) {
            return tokens[tokenIndex];
        } else {
            return _endOfFileToken;
        }
    }

   private:
    const Token _endOfFileToken;
};

class ProductionState {
   public:
    bool cutHasBeenHit;
    CaptureArray captures;

    ProductionState() : cutHasBeenHit(false), captures({}) {}

    std::array<int, kNumCaptures> captureCounts() const {
        std::array<int, kNumCaptures> counts;
        for (auto i = 0; i < kNumCaptures; i++) {
            counts[i] = captures[i]->count();
        }
        return counts;
    }
};

class Checkpoint {
   public:
    Checkpoint(const InputState& inputState, const ProductionState& productionState)
        : _tokenIndex(inputState.tokenIndex),
          _cutHasBeenHit(productionState.cutHasBeenHit),
          _captureCounts(productionState.captureCounts()) {}

    void revert(InputState& inputState, ProductionState& productionState) {
        inputState.tokenIndex = _tokenIndex;
        productionState.cutHasBeenHit = _cutHasBeenHit;
        for (auto i = 0; i < kNumCaptures; i++) {
            trimCaptures(productionState.captures[i].get(), _captureCounts[i]);
        }
    }

   private:
    const int _tokenIndex;
    const bool _cutHasBeenHit;
    const std::array<int, kNumCaptures> _captureCounts;

    void trimCaptures(Box* box, size_t desiredCount) {
        while (box->count() > desiredCount) {
            box->removeAt(box->count() - 1);
        }
    }
};

class TermResult {
   public:
    bool isMatch;
    bool isError;
    const Term* mismatchedTerm;
    std::optional<Token> token;
    std::unique_ptr<Box> box;

    static TermResult success() { return TermResult(true, false, nullptr, {}, {}); }

    static TermResult success(std::unique_ptr<Box> box) { return TermResult(true, false, nullptr, {}, std::move(box)); }

    static TermResult mismatch(const Term& mismatchedTerm, Token token) {
        return TermResult(false, false, &mismatchedTerm, token, nullptr);
    }

    static TermResult error(const Term& mismatchedTerm, Token token) {
        return TermResult(false, true, &mismatchedTerm, token, nullptr);
    }

    static TermResult mismatchOrError(const ProductionState& productionState, const Term& mismatchedTerm, Token token) {
        if (productionState.cutHasBeenHit) {
            return error(mismatchedTerm, token);
        } else {
            return mismatch(mismatchedTerm, token);
        }
    }

   private:
    TermResult(
        bool isMatch,
        bool isError,
        const Term* mismatchedTerm,
        std::optional<Token> token,
        std::unique_ptr<Box> box)
        : isMatch(isMatch), isError(isError), mismatchedTerm(mismatchedTerm), token(token), box(std::move(box)) {}
};

}  // namespace compiler

Parser::Parser() : _productionCollection(std::make_unique<ProductionCollection>()) {}

ParserResult::ParserResult(std::string message, Token token)
    : success(false), message(std::move(message)), token(token) {}

ParserResult::ParserResult(std::unique_ptr<Node> node) : success(true), node(std::move(node)) {}

static Token peekToken(const InputState& inputState) {
    if (inputState.tokenIndex < inputState.tokens.size()) {
        return inputState.tokens[inputState.tokenIndex];
    } else {
        return Token(-1, 0, TokenType::kEndOfFile, "");
    }
}

static void acceptToken(InputState& inputState) {
    inputState.tokenIndex++;
}

static void appendCapture(std::unique_ptr<Box>& existingBox, std::unique_ptr<Box>& newBox) {
    switch (existingBox->type) {
        case BoxType::kNode: {
            auto existingNodeBox = dynamic_cast_borrow<NodeBox>(existingBox);
            auto newNodeBox = dynamic_cast_borrow<NodeBox>(newBox);
            existingNodeBox->append(*newNodeBox);
            break;
        }

        case BoxType::kToken: {
            auto existingTokenBox = dynamic_cast_borrow<TokenBox>(existingBox);
            auto newTokenBox = dynamic_cast_borrow<TokenBox>(newBox);
            existingTokenBox->append(*newTokenBox);
            break;
        }

        default:
            assert(false);
            break;
    }
}

static TermResult parseTerm(const Term& term, InputState& inputState, ProductionState& productionState);

static TermResult parseProductionTerm(const Production& production, InputState& inputState) {
    auto productionState = ProductionState();
    auto checkpoint = Checkpoint(inputState, productionState);
    auto firstToken = peekToken(inputState);

    for (const auto& term : production.terms) {
        auto termResult = parseTerm(term, inputState, productionState);
        if (!termResult.isMatch) {
            checkpoint.revert(inputState, productionState);
            return termResult;
        }
    }

    return TermResult::success(std::move(production.parse(productionState.captures, firstToken)));
}

TermResult parseTerm(const Term& term, InputState& inputState, ProductionState& productionState) {
    auto checkpoint = Checkpoint(inputState, productionState);
    switch (term.type) {
        case TermType::kAnd:
            for (const auto& subTerm : term.subTerms) {
                auto subTermResult = parseTerm(subTerm, inputState, productionState);
                if (!subTermResult.isMatch) {
                    checkpoint.revert(inputState, productionState);
                    return subTermResult;
                }
            }
            return TermResult::success();

        case TermType::kCapture: {
            auto captureResult = parseTerm(term.subTerms[0], inputState, productionState);
            if (captureResult.isMatch) {
                appendCapture(productionState.captures[term.captureId], captureResult.box);
                return TermResult::success();
            } else {
                checkpoint.revert(inputState, productionState);
                return captureResult;
            }
        }

        case TermType::kCut:
            productionState.cutHasBeenHit = true;
            return TermResult::success();

        case TermType::kNonTerminal:
            return parseProductionTerm(*term.production, inputState);

        case TermType::kOptional: {
            for (const auto& subTerm : term.subTerms) {
                auto subTermResult = parseTerm(subTerm, inputState, productionState);
                if (!subTermResult.isMatch) {
                    checkpoint.revert(inputState, productionState);
                    if (subTermResult.isError) {
                        return subTermResult;  // it matched up to the cut, so this is an error
                    } else {
                        return TermResult::success();  // it's ok, this was optional
                    }
                }
            }
            return TermResult::success();
        }

        case TermType::kOr:
            for (const auto& subTerm : term.subTerms) {
                auto subTermResult = parseTerm(subTerm, inputState, productionState);
                if (!subTermResult.isMatch) {
                    checkpoint.revert(inputState, productionState);
                }
                if (subTermResult.isError || subTermResult.isMatch) {
                    return subTermResult;
                }
            }
            return TermResult::mismatchOrError(productionState, term, inputState.currentToken());

        case TermType::kTerminal: {
            auto terminalToken = peekToken(inputState);
            if (term.tokenType == terminalToken.type) {
                acceptToken(inputState);
                return TermResult::success(tokenBox(terminalToken));
            } else {
                checkpoint.revert(inputState, productionState);
                return TermResult::mismatchOrError(productionState, term, inputState.currentToken());
            }
            break;
        }

        case TermType::kZeroOrMore: {
            while (true) {
                for (const auto& subTerm : term.subTerms) {
                    auto subTermResult = parseTerm(subTerm, inputState, productionState);
                    if (!subTermResult.isMatch) {
                        checkpoint.revert(inputState, productionState);
                        if (subTermResult.isError) {
                            return subTermResult;  // it matched up to the cut, so this is an error
                        } else {
                            return TermResult::success();  // it's ok, end of the list
                        }
                    }
                }
            }
        }

        default:
            assert(false);
            return TermResult::success();
    }
}

ParserResult parseProduction(const Production& production, InputState& inputState) {
    auto termResult = parseProductionTerm(production, inputState);
    if (termResult.isMatch) {
        auto nodeBox = dynamic_cast_move<NodeBox>(std::move(termResult.box));
        return ParserResult(std::move(nodeBox->values[0]));
    } else {
        // TODO: better error message here based on the mismatching term
        return ParserResult("This token was unexpected.", termResult.token.value());
    }
}

static ParserResult parseRootProduction(const Production& production, const std::vector<basic::Token>& tokens) {
    auto inputState = InputState(tokens);
    auto result = parseProduction(production, inputState);
    if (result.success && inputState.tokenIndex < inputState.tokens.size()) {
        // the code is good but there is unparsed junk left over
        return ParserResult(
            "This token was unexpected. The block should have ended before this token.",
            inputState.tokens[inputState.tokenIndex]);
    } else {
        return result;
    }
}

ParserResult Parser::parseProgram(const std::vector<basic::Token>& tokens) {
    return parseRootProduction(*_productionCollection->programProduction, tokens);
}

ParserResult Parser::parseMember(const std::vector<basic::Token>& tokens) {
    return parseRootProduction(*_productionCollection->memberProduction, tokens);
}
