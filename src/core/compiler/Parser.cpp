// compile with -DDUMP_PARSE to dump AST parse tree to std::cerr

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
    TokenKind tokenKind;           // when type = kTerminal
    int captureId;                 // when type = kCapture
    Term(TermType type) : type(type), production(nullptr), tokenKind(TokenKind::kError), captureId(-1) {}
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
    std::unique_ptr<Node> value() {
        if (values.size() == 0) {
            return nullptr;
        } else {
            return std::move(values[0]);
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
    Token value() const {
        if (values.size() == 0) {
            return Token();
        } else {
            return values[0];
        }
    }
};

class Production {
   public:
    std::string_view name;  // to ease debugging; not used for any logic
    Term listTerm;
    Production(std::string_view name, std::initializer_list<Term> terms);
    virtual ~Production() {}
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

static Term term(TokenKind TokenKind) {
    auto t = Term(TermType::kTerminal);
    t.tokenKind = TokenKind;
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
    t.subTerms.push_back(list(terms));
    return t;
}

static Term capture(int captureId, Term&& subTerm) {
    auto t = Term(TermType::kCapture);
    t.captureId = captureId;
    t.subTerms.push_back(subTerm);
    return t;
}

static Term cut() {
    return Term(TermType::kCut);
}

Production::Production(std::string_view name, std::initializer_list<Term> terms) : name(name), listTerm(list(terms)) {}

//
// captures
//

template <typename T>
static std::vector<std::unique_ptr<T>> captureNodeArray(std::unique_ptr<Box>& box) {
    if (!box || box->count() == 0) {
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
    return dynamic_cast_move<T>(std::move(nodeBox->value()));
}

static bool hasCapture(std::unique_ptr<Box>& box) {
    return box && box->count() > 0;
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
    return tokenBox->value();
}

static Token captureTokenNoMove(const Box* box) {
    assert(box->type == BoxType::kToken);
    auto tokenBox = dynamic_cast<const TokenBox*>(box);
    return tokenBox->value();
}

static std::string captureTokenText(std::unique_ptr<Box> box) {
    return captureToken(std::move(box)).text;
}

static TokenKind captureTokenKind(std::unique_ptr<Box> box) {
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

static BinaryOperator TokenKindToBinaryOperator(TokenKind TokenKind) {
    switch (TokenKind) {
        case TokenKind::kMultiplicationSign:
            return BinaryOperator::kMultiply;
        case TokenKind::kDivisionSign:
            return BinaryOperator::kDivide;
        case TokenKind::kMod:
            return BinaryOperator::kModulus;
        case TokenKind::kPlusSign:
            return BinaryOperator::kAdd;
        case TokenKind::kMinusSign:
            return BinaryOperator::kSubtract;
        case TokenKind::kLessThanSign:
            return BinaryOperator::kLessThan;
        case TokenKind::kLessThanEqualsSign:
            return BinaryOperator::kLessThanEquals;
        case TokenKind::kGreaterThanSign:
            return BinaryOperator::kGreaterThan;
        case TokenKind::kGreaterThanEqualsSign:
            return BinaryOperator::kGreaterThanEquals;
        case TokenKind::kEqualsSign:
            return BinaryOperator::kEquals;
        case TokenKind::kNotEqualsSign:
            return BinaryOperator::kNotEquals;
        case TokenKind::kAnd:
            return BinaryOperator::kAnd;
        case TokenKind::kOr:
            return BinaryOperator::kOr;
        default:
            assert(false);
            return {};
    }
}

static std::unique_ptr<Box> parseBinaryExpressionSuffix(CaptureArray& captures, const Token& firstToken) {
    return nodeBox(new BinaryExpressionSuffixNode(
        TokenKindToBinaryOperator(captureTokenKind(std::move(captures[0]))),
        captureSingleNode<ExpressionNode>(std::move(captures[1])), firstToken));
}

static std::unique_ptr<Box> parseBinaryExpression(CaptureArray& captures, const Token& firstToken) {
    if (hasCapture(captures[1])) {
        auto token = captureTokenNoMove(captures[1].get());
        return nodeBox(new BinaryExpressionNode(
            captureSingleNode<ExpressionNode>(std::move(captures[0])),
            captureNodeArray<BinaryExpressionSuffixNode>(captures[1]), token));
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
        : Production(
              NAMEOF_TYPE(BodyProduction),
              {
                  zeroOrMore({
                      capture(0, prod(statement)),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto statements = captureNodeArray<StatementNode>(captures[0]);
        auto body = new BodyNode(statements, firstToken);
        return nodeBox(body);
    }
};

class ArgumentListProduction : public Production {
   public:
    ArgumentListProduction(const Production* expression)
        : Production(
              NAMEOF_TYPE(ArgumentListProduction),
              {
                  optional({
                      capture(0, prod(expression)),
                      zeroOrMore({
                          term(TokenKind::kComma),
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
        : Production(
              NAMEOF_TYPE(ParameterProduction),
              {
                  capture(0, term(TokenKind::kIdentifier)),
                  cut(),
                  term(TokenKind::kAs),
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
        : Production(
              NAMEOF_TYPE(ParameterListProduction),
              {
                  optional({
                      capture(0, prod(parameter)),
                      cut(),
                      zeroOrMore({
                          term(TokenKind::kComma),
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
        : Production(
              NAMEOF_TYPE(NamedTypeProduction),
              {
                  capture(0, term(TokenKind::kIdentifier)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto type = new TypeNode(Kind::kRecord, firstToken);
        type->recordName = std::optional<std::string>(captureTokenText(std::move(captures[0])));
        return nodeBox(type);
    }
};

class TypeWithParenthesesProduction : public Production {
   public:
    TypeWithParenthesesProduction(const Production* type)
        : Production(
              NAMEOF_TYPE(TypeWithParenthesesProduction),
              {
                  oneOf({
                      list({
                          term(TokenKind::kLeftParenthesis),
                          cut(),
                          capture(0, prod(type)),
                          term(TokenKind::kRightParenthesis),
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
        : Production(
              NAMEOF_TYPE(OptionalTypeProduction),
              {
                  term(TokenKind::kOptional),
                  cut(),
                  capture(0, prod(typeWithParentheses)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto type = new TypeNode(Kind::kOptional, firstToken);
        type->optionalValueType = captureSingleNode<TypeNode>(std::move(captures[0]));
        return nodeBox(type);
    }
};

class MapTypeProduction : public Production {
   public:
    MapTypeProduction(const Production* typeWithParentheses)
        : Production(
              NAMEOF_TYPE(MapTypeProduction),
              {
                  term(TokenKind::kMap),
                  cut(),
                  term(TokenKind::kFrom),
                  capture(0, prod(typeWithParentheses)),
                  term(TokenKind::kTo),
                  capture(1, prod(typeWithParentheses)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto type = new TypeNode(Kind::kMap, firstToken);
        type->mapKeyType = captureSingleNode<TypeNode>(std::move(captures[0]));
        type->mapValueType = captureSingleNode<TypeNode>(std::move(captures[1]));
        return nodeBox(type);
    }
};

class ListTypeProduction : public Production {
   public:
    ListTypeProduction(const Production* typeWithParentheses)
        : Production(
              NAMEOF_TYPE(ListTypeProduction),
              {
                  term(TokenKind::kList),
                  cut(),
                  term(TokenKind::kOf),
                  capture(0, prod(typeWithParentheses)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto type = new TypeNode(Kind::kList, firstToken);
        type->listItemType = captureSingleNode<TypeNode>(std::move(captures[0]));
        return nodeBox(type);
    }
};

class RecordTypeProduction : public Production {
   public:
    RecordTypeProduction(const Production* parameterList)
        : Production(
              NAMEOF_TYPE(RecordTypeProduction),
              {
                  term(TokenKind::kRecord),
                  cut(),
                  term(TokenKind::kLeftParenthesis),
                  capture(0, prod(parameterList)),
                  term(TokenKind::kRightParenthesis),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto type = new TypeNode(Kind::kRecord, firstToken);
        for (auto& parameter : captureNodeArray<ParameterNode>(captures[0])) {
            auto field = new FieldNode(parameter->name, std::move(parameter->type), firstToken);
            type->fields.push_back(std::unique_ptr<FieldNode>(field));
        }
        return nodeBox(type);
    }
};

class PrimitiveTypeProduction : public Production {
   public:
    PrimitiveTypeProduction()
        : Production(
              NAMEOF_TYPE(PrimitiveTypeProduction),
              {
                  capture(
                      0,
                      oneOf({
                          term(TokenKind::kBoolean),
                          term(TokenKind::kNumber),
                          term(TokenKind::kText),
                          term(TokenKind::kDate),
                          term(TokenKind::kDateTime),
                          term(TokenKind::kDateTimeOffset),
                          term(TokenKind::kTimeSpan),
                      })),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        Kind k;
        switch (captureTokenKind(std::move(captures[0]))) {
            case TokenKind::kBoolean:
                k = Kind::kBoolean;
                break;
            case TokenKind::kNumber:
                k = Kind::kNumber;
                break;
            case TokenKind::kText:
                k = Kind::kText;
                break;
            case TokenKind::kDate:
                k = Kind::kDate;
                break;
            case TokenKind::kDateTime:
                k = Kind::kDateTime;
                break;
            case TokenKind::kDateTimeOffset:
                k = Kind::kDateTimeOffset;
                break;
            case TokenKind::kTimeSpan:
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
    TypeProduction() : Production(NAMEOF_TYPE(TypeProduction), {}) {}

    void init(
        const Production* primitiveType,
        const Production* recordType,
        const Production* listType,
        const Production* mapType,
        const Production* optionalType,
        const Production* namedType) {
        listTerm = list({
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
        });
    }

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class LiteralValueProduction : public Production {
   public:
    LiteralValueProduction()
        : Production(
              NAMEOF_TYPE(LiteralValueProduction),
              {
                  capture(
                      0,
                      oneOf({
                          term(TokenKind::kBooleanLiteral),
                          term(TokenKind::kNumberLiteral),
                          term(TokenKind::kStringLiteral),
                      })),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto token = captureToken(std::move(captures[0]));
        switch (token.type) {
            case TokenKind::kBooleanLiteral: {
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

            case TokenKind::kNumberLiteral:
                return nodeBox(new LiteralNumberExpressionNode(parseDecimalString(token.text), firstToken));

            case TokenKind::kStringLiteral: {
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
        : Production(
              NAMEOF_TYPE(LiteralRecordFieldProduction),
              {
                  capture(0, term(TokenKind::kIdentifier)),
                  cut(),
                  term(TokenKind::kColon),
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
        : Production(
              NAMEOF_TYPE(LiteralRecordFieldListProduction),
              {
                  optional({
                      capture(0, prod(literalRecordField)),
                      cut(),
                      zeroOrMore({
                          term(TokenKind::kComma),
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
        : Production(
              NAMEOF_TYPE(LiteralRecordTermProduction),
              {
                  term(TokenKind::kLeftBrace),
                  cut(),
                  capture(0, prod(literalRecordFieldList)),
                  term(TokenKind::kRightBrace),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(
            new LiteralRecordExpressionNode(captureNodeArray<LiteralRecordFieldNode>(captures[0]), firstToken));
    }
};

class LiteralArrayTermProduction : public Production {
   public:
    LiteralArrayTermProduction(const Production* argumentList)
        : Production(
              NAMEOF_TYPE(LiteralArrayTermProduction),
              {
                  term(TokenKind::kLeftBracket),
                  cut(),
                  capture(0, prod(argumentList)),
                  term(TokenKind::kRightBracket),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new LiteralArrayExpressionNode(captureNodeArray<ExpressionNode>(captures[0]), firstToken));
    }
};

class FunctionCallTermProduction : public Production {
   public:
    FunctionCallTermProduction(const Production* argumentList)
        : Production(
              NAMEOF_TYPE(FunctionCallTermProduction),
              {
                  capture(0, term(TokenKind::kIdentifier)),
                  term(TokenKind::kLeftParenthesis),
                  cut(),
                  capture(1, prod(argumentList)),
                  term(TokenKind::kRightParenthesis),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new CallExpressionNode(
            captureTokenText(std::move(captures[0])), captureNodeArray<ExpressionNode>(captures[1]), firstToken));
    }
};

class ParenthesesTermProduction : public Production {
   public:
    ParenthesesTermProduction(const Production* expression)
        : Production(
              NAMEOF_TYPE(ParenthesesTermProduction),
              {
                  term(TokenKind::kLeftParenthesis),
                  cut(),
                  capture(0, prod(expression)),
                  term(TokenKind::kRightParenthesis),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
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
        : Production(
              NAMEOF_TYPE(ExpressionTermProduction),
              {
                  oneOf({
                      capture(0, prod(literalValue)),
                      capture(0, prod(parenthesesTerm)),
                      capture(0, prod(functionCallTerm)),
                      capture(0, prod(literalArrayTerm)),
                      capture(0, prod(literalRecordTerm)),
                      capture(1, term(TokenKind::kIdentifier)),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        if (hasCapture(captures[0])) {
            return std::move(captures[0]);
        } else if (hasCapture(captures[1])) {
            return nodeBox(new SymbolReferenceExpressionNode(captureTokenText(std::move(captures[1])), firstToken));
        } else {
            assert(false);
            return {};
        }
    }
};

class DottedExpressionSuffixProduction : public Production {
   public:
    DottedExpressionSuffixProduction(const Production* argumentList)
        : Production(
              NAMEOF_TYPE(DottedExpressionSuffixProduction),
              {
                  term(TokenKind::kDot),
                  cut(),
                  capture(0, term(TokenKind::kIdentifier)),
                  optional({
                      term(TokenKind::kLeftParenthesis),
                      capture(1, prod(argumentList)),
                      term(TokenKind::kRightParenthesis),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto isCall = hasCapture(captures[1]);
        return nodeBox(new DottedExpressionSuffixNode(
            captureTokenText(std::move(captures[0])), isCall, captureNodeArray<ExpressionNode>(captures[1]),
            firstToken));
    }
};

class DottedExpressionProduction : public Production {
   public:
    DottedExpressionProduction(const Production* expressionTerm, const Production* dottedExpressionSuffix)
        : Production(
              NAMEOF_TYPE(DottedExpressionProduction),
              {
                  capture(0, prod(expressionTerm)),
                  zeroOrMore({
                      capture(1, prod(dottedExpressionSuffix)),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto dottedSuffixes = captureNodeArray<DottedExpressionSuffixNode>(captures[1]);
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
        : Production(
              NAMEOF_TYPE(ConvertExpressionProduction),
              {
                  capture(0, prod(dottedExpression)),
                  optional({
                      term(TokenKind::kAs),
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
        : Production(
              NAMEOF_TYPE(UnaryExpressionProduction),
              {
                  optional({
                      capture(0, term(TokenKind::kNot)),
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
        : Production(
              NAMEOF_TYPE(MultiplyExpressionSuffixProduction),
              {
                  capture(
                      0,
                      oneOf({
                          term(TokenKind::kMultiplicationSign),
                          term(TokenKind::kDivisionSign),
                          term(TokenKind::kMod),
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
        : Production(
              NAMEOF_TYPE(MultiplyExpressionProduction),
              {
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
        : Production(
              NAMEOF_TYPE(AddExpressionSuffixProduction),
              {
                  capture(
                      0,
                      oneOf({
                          term(TokenKind::kPlusSign),
                          term(TokenKind::kMinusSign),
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
        : Production(
              NAMEOF_TYPE(AddExpressionProduction),
              {
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
        : Production(
              NAMEOF_TYPE(InequalityExpressionSuffixProduction),
              {
                  capture(
                      0,
                      oneOf({
                          term(TokenKind::kLessThanSign),
                          term(TokenKind::kLessThanEqualsSign),
                          term(TokenKind::kGreaterThanSign),
                          term(TokenKind::kGreaterThanEqualsSign),
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
        : Production(
              NAMEOF_TYPE(InequalityExpressionProduction),
              {
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
        : Production(
              NAMEOF_TYPE(EqualityExpressionSuffixProduction),
              {
                  capture(
                      0,
                      oneOf({
                          term(TokenKind::kEqualsSign),
                          term(TokenKind::kNotEqualsSign),
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
        : Production(
              NAMEOF_TYPE(EqualityExpressionProduction),
              {
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
        : Production(
              NAMEOF_TYPE(AndExpressionSuffixProduction),
              {
                  capture(0, term(TokenKind::kAnd)),
                  capture(1, prod(equalityExpression)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return parseBinaryExpressionSuffix(captures, firstToken);
    }
};

class AndExpressionProduction : public Production {
   public:
    AndExpressionProduction(const Production* equalityExpression, const Production* andExpressionSuffix)
        : Production(
              NAMEOF_TYPE(AndExpressionProduction),
              {
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
        : Production(
              NAMEOF_TYPE(OrExpressionSuffixProduction),
              {
                  capture(0, term(TokenKind::kOr)),
                  capture(1, prod(andExpression)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return parseBinaryExpressionSuffix(captures, firstToken);
    }
};

class OrExpressionProduction : public Production {
   public:
    OrExpressionProduction(const Production* andExpression, const Production* orExpressionSuffix)
        : Production(
              NAMEOF_TYPE(OrExpressionProduction),
              {
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
    ExpressionProduction() : Production(NAMEOF_TYPE(ExpressionProduction), {}) {}

    void init(const Production* orExpression) {
        listTerm = list({
            capture(0, prod(orExpression)),
        });
    }

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class FinallyBlockProduction : public Production {
   public:
    FinallyBlockProduction(const Production* body)
        : Production(
              NAMEOF_TYPE(FinallyBlockProduction),
              {
                  term(TokenKind::kFinally),
                  cut(),
                  term(TokenKind::kEndOfLine),
                  capture(0, prod(body)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class CatchBlockProduction : public Production {
   public:
    CatchBlockProduction(const Production* body)
        : Production(
              NAMEOF_TYPE(CatchBlockProduction),
              {
                  term(TokenKind::kCatch),
                  cut(),
                  term(TokenKind::kEndOfLine),
                  capture(0, prod(body)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class TryStatementProduction : public Production {
   public:
    TryStatementProduction(const Production* body, const Production* catchBlock, const Production* finallyBlock)
        : Production(
              NAMEOF_TYPE(TryStatementProduction),
              {
                  term(TokenKind::kTry),
                  cut(),
                  term(TokenKind::kEndOfLine),
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
        : Production(
              NAMEOF_TYPE(RethrowStatementProduction),
              {
                  term(TokenKind::kRethrow),
                  cut(),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new RethrowStatementNode(firstToken));
    }
};

class ThrowStatementProduction : public Production {
   public:
    ThrowStatementProduction(const Production* expression)
        : Production(
              NAMEOF_TYPE(ThrowStatementProduction),
              {
                  term(TokenKind::kThrow),
                  cut(),
                  capture(0, prod(expression)),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new ThrowStatementNode(captureSingleNode<ExpressionNode>(std::move(captures[0])), firstToken));
    }
};

class ExitStatementProduction : public Production {
   public:
    ExitStatementProduction()
        : Production(
              NAMEOF_TYPE(ExitStatementProduction),
              {
                  term(TokenKind::kExit),
                  capture(
                      0,
                      oneOf({
                          term(TokenKind::kDo),
                          term(TokenKind::kFor),
                          term(TokenKind::kSelect),
                          term(TokenKind::kTry),
                          term(TokenKind::kWhile),
                      })),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        ExitScope scope;
        switch (captureTokenKind(std::move(captures[0]))) {
            case TokenKind::kDo:
                scope = ExitScope::kDo;
                break;
            case TokenKind::kFor:
                scope = ExitScope::kFor;
                break;
            case TokenKind::kSelect:
                scope = ExitScope::kSelectCase;
                break;
            case TokenKind::kTry:
                scope = ExitScope::kTry;
                break;
            case TokenKind::kWhile:
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
        : Production(
              NAMEOF_TYPE(ContinueStatementProduction),
              {
                  term(TokenKind::kExit),
                  capture(
                      0,
                      oneOf({
                          term(TokenKind::kDo),
                          term(TokenKind::kFor),
                          term(TokenKind::kWhile),
                      })),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        ContinueScope scope;
        switch (captureTokenKind(std::move(captures[0]))) {
            case TokenKind::kDo:
                scope = ContinueScope::kDo;
                break;
            case TokenKind::kFor:
                scope = ContinueScope::kFor;
                break;
            case TokenKind::kWhile:
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
        : Production(
              NAMEOF_TYPE(CallStatementProduction),
              {
                  capture(0, term(TokenKind::kIdentifier)),
                  cut(),
                  capture(1, prod(argumentList)),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new CallStatementNode(
            captureTokenText(std::move(captures[0])), captureNodeArray<ExpressionNode>(captures[1]), firstToken));
    }
};

class ReturnStatementProduction : public Production {
   public:
    ReturnStatementProduction(const Production* expression)
        : Production(
              NAMEOF_TYPE(ReturnStatementProduction),
              {
                  term(TokenKind::kReturn),
                  cut(),
                  optional({
                      capture(0, prod(expression)),
                  }),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(
            new ReturnStatementNode(captureSingleNodeOrNull<ExpressionNode>(std::move(captures[0])), firstToken));
    }
};

class SelectStatementProduction : public Production {
   public:
    SelectStatementProduction(const Production* expression)
        : Production(
              NAMEOF_TYPE(SelectStatementProduction),
              {
                  term(TokenKind::kSelect),
                  // no cut here, since it could be 'select' 'case'
                  capture(0, prod(expression)),
                  cut(),  // instead the cut is here
                  optional({
                      term(TokenKind::kTo),
                      capture(1, prod(expression)),
                  }),
                  term(TokenKind::kEndOfLine),
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
        : Production(
              NAMEOF_TYPE(ConstStatementProduction),
              {
                  term(TokenKind::kConst),
                  cut(),
                  capture(0, term(TokenKind::kIdentifier)),
                  term(TokenKind::kEqualsSign),
                  capture(1, prod(literalValue)),
                  term(TokenKind::kEndOfLine),
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
        : Production(
              NAMEOF_TYPE(AssignLocationSuffixProduction),
              {
                  oneOf({
                      list({
                          term(TokenKind::kDot),
                          capture(0, term(TokenKind::kIdentifier)),
                      }),
                      list({
                          term(TokenKind::kLeftParenthesis),
                          capture(1, prod(expression)),
                          term(TokenKind::kRightParenthesis),
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
        : Production(
              NAMEOF_TYPE(AssignStatementProduction),
              {
                  capture(0, term(TokenKind::kIdentifier)),
                  zeroOrMore({
                      capture(1, prod(assignLocationSuffix)),
                  }),
                  term(TokenKind::kEqualsSign),
                  capture(2, prod(expression)),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new AssignStatementNode(
            captureTokenText(std::move(captures[0])), captureNodeArray<AssignLocationSuffixNode>(captures[1]),
            captureSingleNode<ExpressionNode>(std::move(captures[2])), firstToken));
    }
};

class DimStatementProduction : public Production {
   public:
    DimStatementProduction(const Production* type, const Production* expression)
        : Production(
              NAMEOF_TYPE(DimStatementProduction),
              {
                  term(TokenKind::kDim),  // no cut here because it could be "dim list"
                  capture(0, term(TokenKind::kIdentifier)),
                  cut(),  // cut here instead
                  oneOf({
                      list({
                          term(TokenKind::kAs),
                          capture(1, prod(type)),
                      }),
                      list({
                          term(TokenKind::kEqualsSign),
                          capture(2, prod(expression)),
                      }),
                  }),
                  term(TokenKind::kEndOfLine),
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
        : Production(
              NAMEOF_TYPE(DimCollectionStatementProduction),
              {
                  term(TokenKind::kDim),
                  capture(
                      0,
                      oneOf({
                          term(TokenKind::kList),
                          term(TokenKind::kMap),
                      })),
                  capture(1, term(TokenKind::kIdentifier)),
                  term(TokenKind::kEndOfLine),
                  capture(2, prod(body)),
                  term(TokenKind::kEnd),
                  term(TokenKind::kDim),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        CollectionType type;
        switch (captureTokenKind(std::move(captures[0]))) {
            case TokenKind::kList:
                type = CollectionType::kList;
                break;
            case TokenKind::kMap:
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
        : Production(
              NAMEOF_TYPE(CaseValueProduction),
              {
                  capture(0, prod(expression)),
                  optional({
                      term(TokenKind::kTo),
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
        : Production(
              NAMEOF_TYPE(CaseValueListProduction),
              {
                  capture(0, prod(caseValue)),
                  zeroOrMore({
                      term(TokenKind::kComma),
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
        : Production(
              NAMEOF_TYPE(CaseProduction),
              {
                  term(TokenKind::kCase),
                  cut(),
                  oneOf({
                      capture(0, prod(caseValueList)),
                      term(TokenKind::kElse),
                  }),
                  term(TokenKind::kEndOfLine),
                  capture(1, prod(body)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new CaseNode(
            captureNodeArray<CaseValueNode>(captures[0]), captureSingleNode<BodyNode>(std::move(captures[1])),
            firstToken));
    }
};

class SelectCaseStatementProduction : public Production {
   public:
    SelectCaseStatementProduction(const Production* expression, const Production* case_)
        : Production(
              NAMEOF_TYPE(SelectCaseStatementProduction),
              {
                  term(TokenKind::kSelect),
                  term(TokenKind::kCase),
                  cut(),
                  capture(0, prod(expression)),
                  term(TokenKind::kEndOfLine),
                  zeroOrMore({
                      capture(1, prod(case_)),
                  }),
                  term(TokenKind::kEnd),
                  term(TokenKind::kSelect),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new SelectCaseStatementNode(
            captureSingleNode<ExpressionNode>(std::move(captures[0])), captureNodeArray<CaseNode>(captures[1]),
            firstToken));
    }
};

class GroupStatementProduction : public Production {
   public:
    GroupStatementProduction(const Production* expression, const Production* body)
        : Production(
              NAMEOF_TYPE(GroupStatementProduction),
              {
                  term(TokenKind::kGroup),
                  cut(),
                  capture(0, prod(expression)),
                  term(TokenKind::kBy),
                  capture(1, prod(expression)),
                  term(TokenKind::kInto),
                  capture(2, term(TokenKind::kIdentifier)),
                  optional({
                      term(TokenKind::kWith),
                      term(TokenKind::kKey),
                      capture(3, term(TokenKind::kIdentifier)),
                  }),
                  term(TokenKind::kEndOfLine),
                  capture(4, prod(body)),
                  term(TokenKind::kNext),
                  term(TokenKind::kEndOfLine),
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
        : Production(
              NAMEOF_TYPE(JoinStatementProduction),
              {
                  term(TokenKind::kJoin),
                  cut(),
                  capture(0, term(TokenKind::kIdentifier)),
                  term(TokenKind::kIn),
                  capture(1, prod(expression)),
                  term(TokenKind::kOn),
                  capture(2, prod(expression)),
                  term(TokenKind::kEndOfLine),
                  capture(1, prod(body)),
                  term(TokenKind::kNext),
                  term(TokenKind::kEndOfLine),
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
        : Production(
              NAMEOF_TYPE(DoConditionProduction),
              {
                  capture(
                      0,
                      oneOf({
                          term(TokenKind::kWhile),
                          term(TokenKind::kUntil),
                      })),
                  cut(),
                  capture(1, prod(expression)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        DoConditionType type;
        switch (captureTokenKind(std::move(captures[0]))) {
            case TokenKind::kWhile:
                type = DoConditionType::kWhile;
                break;
            case TokenKind::kUntil:
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
        : Production(
              NAMEOF_TYPE(DoStatementProduction),
              {
                  term(TokenKind::kDo),
                  cut(),
                  oneOf({
                      list({
                          capture(0, prod(doCondition)),
                          term(TokenKind::kEndOfLine),
                          capture(1, prod(body)),
                          term(TokenKind::kLoop),
                      }),
                      list({
                          term(TokenKind::kEndOfLine),
                          capture(1, prod(body)),
                          term(TokenKind::kLoop),
                          capture(2, prod(doCondition)),
                      }),
                  }),
                  term(TokenKind::kEndOfLine),
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
        : Production(
              NAMEOF_TYPE(ElseIfProduction),
              {
                  term(TokenKind::kElse),
                  term(TokenKind::kIf),
                  capture(0, prod(expression)),
                  term(TokenKind::kThen),
                  term(TokenKind::kEndOfLine),
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
        : Production(
              NAMEOF_TYPE(IfStatementProduction),
              {
                  term(TokenKind::kIf),
                  cut(),
                  capture(0, prod(expression)),
                  term(TokenKind::kThen),
                  term(TokenKind::kEndOfLine),
                  capture(1, prod(body)),
                  zeroOrMore({
                      capture(2, prod(elseIf)),
                  }),
                  optional({
                      term(TokenKind::kElse),
                      term(TokenKind::kEndOfLine),
                      capture(3, prod(body)),
                  }),
                  term(TokenKind::kEnd),
                  term(TokenKind::kIf),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new IfStatementNode(
            captureSingleNode<ExpressionNode>(std::move(captures[0])),
            captureSingleNode<BodyNode>(std::move(captures[1])), captureNodeArray<ElseIfNode>(captures[2]),
            captureSingleNodeOrNull<BodyNode>(std::move(captures[3])), firstToken));
    }
};

class WhileStatementProduction : public Production {
   public:
    WhileStatementProduction(const Production* expression, const Production* body)
        : Production(
              NAMEOF_TYPE(WhileStatementProduction),
              {
                  term(TokenKind::kWhile),
                  cut(),
                  capture(0, prod(expression)),
                  term(TokenKind::kEndOfLine),
                  capture(1, prod(body)),
                  term(TokenKind::kWend),
                  term(TokenKind::kEndOfLine),
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
        : Production(
              NAMEOF_TYPE(ForEachStatementProduction),
              {
                  term(TokenKind::kFrom),
                  cut(),
                  capture(0, term(TokenKind::kIdentifier)),
                  term(TokenKind::kIn),
                  capture(1, prod(expression)),
                  term(TokenKind::kEndOfLine),
                  capture(2, prod(body)),
                  term(TokenKind::kNext),
                  term(TokenKind::kEndOfLine),
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
        : Production(
              NAMEOF_TYPE(ForStepProduction),
              {
                  term(TokenKind::kStep),
                  cut(),
                  capture(
                      0,
                      oneOf({
                          term(TokenKind::kNumberLiteral),
                          term(TokenKind::kIdentifier),
                      })),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        auto t = captureToken(std::move(captures[0]));
        switch (t.type) {
            case TokenKind::kNumberLiteral:
                return nodeBox(new ForStepNode(parseDecimalString(t.text), firstToken));
            case TokenKind::kIdentifier:
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
        : Production(
              NAMEOF_TYPE(ForStatementProduction),
              {
                  term(TokenKind::kFor),  // no cut here because it could be FOR EACH
                  capture(0, term(TokenKind::kIdentifier)),
                  cut(),
                  term(TokenKind::kEqualsSign),
                  capture(1, prod(expression)),
                  term(TokenKind::kTo),
                  capture(2, prod(expression)),
                  optional({
                      capture(3, prod(forStep)),
                  }),
                  term(TokenKind::kEndOfLine),
                  capture(4, prod(body)),
                  term(TokenKind::kNext),
                  term(TokenKind::kEndOfLine),
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
        : Production(
              NAMEOF_TYPE(CommandStatementProduction),
              {
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
    StatementProduction() : Production(NAMEOF_TYPE(StatementProduction), {}) {}

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
        listTerm = list({
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
        });
    }

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return std::move(captures[0]);
    }
};

class TypeDeclarationProduction : public Production {
   public:
    TypeDeclarationProduction(const Production* parameter)
        : Production(
              NAMEOF_TYPE(TypeDeclarationProduction),
              {
                  term(TokenKind::kType),
                  cut(),
                  capture(0, term(TokenKind::kIdentifier)),
                  term(TokenKind::kEndOfLine),
                  zeroOrMore({
                      capture(1, prod(parameter)),
                      term(TokenKind::kEndOfLine),
                  }),
                  term(TokenKind::kEnd),
                  term(TokenKind::kType),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new TypeDeclarationNode(
            captureTokenText(std::move(captures[0])), captureNodeArray<ParameterNode>(captures[1]), firstToken));
    }
};

class FunctionProduction : public Production {
   public:
    FunctionProduction(const Production* parameterList, const Production* type, const Production* body)
        : Production(
              NAMEOF_TYPE(FunctionProduction),
              {
                  term(TokenKind::kFunction),
                  cut(),
                  capture(0, term(TokenKind::kIdentifier)),
                  term(TokenKind::kLeftParenthesis),
                  capture(1, prod(parameterList)),
                  term(TokenKind::kRightParenthesis),
                  term(TokenKind::kAs),
                  capture(2, prod(type)),
                  term(TokenKind::kEndOfLine),
                  capture(3, prod(body)),
                  term(TokenKind::kEnd),
                  term(TokenKind::kFunction),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new ProcedureNode(
            captureTokenText(std::move(captures[0])), captureNodeArray<ParameterNode>(captures[1]),
            captureSingleNode<TypeNode>(std::move(captures[2])), captureSingleNode<BodyNode>(std::move(captures[3])),
            firstToken));
    }
};

class SubroutineProduction : public Production {
   public:
    SubroutineProduction(const Production* parameterList, const Production* body)
        : Production(
              NAMEOF_TYPE(SubroutineProduction),
              {
                  term(TokenKind::kSub),
                  cut(),
                  capture(0, term(TokenKind::kIdentifier)),
                  term(TokenKind::kLeftParenthesis),
                  capture(1, prod(parameterList)),
                  term(TokenKind::kRightParenthesis),
                  term(TokenKind::kEndOfLine),
                  capture(2, prod(body)),
                  term(TokenKind::kEnd),
                  term(TokenKind::kSub),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new ProcedureNode(
            captureTokenText(std::move(captures[0])), captureNodeArray<ParameterNode>(captures[1]),
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
        : Production(
              NAMEOF_TYPE(MemberProduction),
              {
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
        : Production(
              NAMEOF_TYPE(ProgramProduction),
              {
                  zeroOrMore({
                      capture(0, prod(member)),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray& captures, const Token& firstToken) const override {
        return nodeBox(new ProgramNode(captureNodeArray<Node>(captures[0]), firstToken));
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
        : tokens(tokens), tokenIndex(0), _endOfFileToken(Token(-1, 0, TokenKind::kEndOfFile, "")) {}

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
    Token firstToken;

    ProductionState(Token firstToken) : cutHasBeenHit(false), firstToken(firstToken) {
        for (auto i = 0; i < kNumCaptures; i++) {
            captures[i] = nullptr;
        }
    }

    std::array<int, kNumCaptures> captureCounts() const {
        std::array<int, kNumCaptures> counts;
        for (auto i = 0; i < kNumCaptures; i++) {
            if (captures[i]) {
                counts[i] = captures[i]->count();
            } else {
                counts[i] = 0;
            }
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
        if (box) {
            while (box->count() > desiredCount) {
                box->removeAt(box->count() - 1);
            }
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

    static std::unique_ptr<TermResult> newSuccess() {
        return std::make_unique<TermResult>(true, false, nullptr, std::optional<Token>(), std::unique_ptr<Box>());
    }

    static std::unique_ptr<TermResult> newSuccess(std::unique_ptr<Box> box) {
        return std::make_unique<TermResult>(true, false, nullptr, std::optional<Token>(), std::move(box));
    }

    static std::unique_ptr<TermResult> newMismatch(const Term& mismatchedTerm, Token token) {
        return std::make_unique<TermResult>(false, false, &mismatchedTerm, token, std::unique_ptr<Box>());
    }

    static std::unique_ptr<TermResult> newError(const Term& mismatchedTerm, Token token) {
        return std::make_unique<TermResult>(false, true, &mismatchedTerm, token, std::unique_ptr<Box>());
    }

    static std::unique_ptr<TermResult> newMismatchOrError(
        const ProductionState& productionState,
        const Term& mismatchedTerm,
        Token token) {
        if (productionState.cutHasBeenHit) {
            return newError(mismatchedTerm, token);
        } else {
            return newMismatch(mismatchedTerm, token);
        }
    }

    TermResult(
        bool isMatch,
        bool isError,
        const Term* mismatchedTerm,
        std::optional<Token> token,
        std::unique_ptr<Box> box)
        : isMatch(isMatch), isError(isError), mismatchedTerm(mismatchedTerm), token(token), box(std::move(box)) {}
};

class ParseStackFrame {
   public:
    // either term or production
    const Term* term = nullptr;
    const Production* production = nullptr;

    std::unique_ptr<ProductionState> productionState = nullptr;  // when production != nullptr
    ProductionState* productionStateRef = nullptr;               // when production == nullptr
    std::unique_ptr<Checkpoint> checkpoint = nullptr;
    std::unique_ptr<TermResult> subTermResult = nullptr;
    bool previousCutStatus = false;                           // when term->type == kOptional or kZeroOrMore
    std::unique_ptr<Checkpoint> elementCheckpoint = nullptr;  // when term->type == kZeroOrMore

    size_t step = 0;

    ParseStackFrame() : productionState(std::make_unique<ProductionState>(Token(0, 0, TokenKind::kEndOfFile, ""))) {
        productionStateRef = productionState.get();
    }
    ParseStackFrame(
        const Production& production,
        InputState& inputState,
        std::unique_ptr<ProductionState> takeProductionState)
        : production(&production), checkpoint(std::make_unique<Checkpoint>(inputState, *takeProductionState)) {
        productionState = std::move(takeProductionState);
        productionStateRef = productionState.get();
    }
    ParseStackFrame(const Term& term, InputState& inputState, ProductionState* productionState)
        : term(&term),
          checkpoint(std::make_unique<Checkpoint>(inputState, *productionState)),
          productionStateRef(productionState) {}
};

Parser::Parser() : _productionCollection(std::make_unique<ProductionCollection>()) {}

Parser::~Parser() {}

ParserResult::ParserResult(std::string message, Token token)
    : success(false), message(std::move(message)), token(token) {}

ParserResult::ParserResult(std::unique_ptr<Node> node) : success(true), node(std::move(node)) {}

static Token peekToken(const InputState& inputState) {
    if (inputState.tokenIndex < inputState.tokens.size()) {
        return inputState.tokens[inputState.tokenIndex];
    } else {
        return Token(-1, 0, TokenKind::kEndOfFile, "");
    }
}

static void acceptToken(InputState& inputState) {
    inputState.tokenIndex++;
}

static void appendCapture(std::unique_ptr<Box>& existingBox, std::unique_ptr<Box>& newBox) {
    if (existingBox) {
        switch (existingBox->type) {
            case BoxType::kNode: {
                if (!existingBox) {
                    existingBox = std::make_unique<NodeBox>();
                }
                auto existingNodeBox = dynamic_cast_borrow<NodeBox>(existingBox);
                auto newNodeBox = dynamic_cast_borrow<NodeBox>(newBox);
                existingNodeBox->append(*newNodeBox);
                break;
            }

            case BoxType::kToken: {
                if (!existingBox) {
                    existingBox = std::make_unique<TokenBox>();
                }
                auto existingTokenBox = dynamic_cast_borrow<TokenBox>(existingBox);
                auto newTokenBox = dynamic_cast_borrow<TokenBox>(newBox);
                existingTokenBox->append(*newTokenBox);
                break;
            }

            default:
                assert(false);
                break;
        }
    } else {
        existingBox = std::move(newBox);
    }
}

static void pumpParseAnd(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>& stack,
    InputState& inputState) {
    // step 0: start subTerms[0]
    // step 1: receive subTerms[0], start subTerms[1]
    // ...
    // step N: receive subTerms[n-1], return

    if (frame->step > 0) {
        // receive subTerms[step-1]
        if (!frame->subTermResult->isMatch) {
            auto result = std::move(frame->subTermResult);
            frame->checkpoint->revert(inputState, *frame->productionStateRef);
            stack.pop();
            stack.top()->subTermResult = std::move(result);
            return;
        }
    }

    if (frame->step < frame->term->subTerms.size()) {
        // start subTerms[step]
        const auto& subTerm = frame->term->subTerms[frame->step];
        stack.push(std::make_unique<ParseStackFrame>(subTerm, inputState, frame->productionStateRef));
        frame->subTermResult = nullptr;
        frame->step++;
    } else {
        // return
        stack.pop();
        stack.top()->subTermResult = TermResult::newSuccess();
    }
}

static void pumpParseCapture(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>& stack,
    InputState& inputState) {
    // step 0: start subTerms[0]
    // step 1: receive subTerms[0], return

    if (frame->step == 0) {
        // start subTerms[0]
        const auto& subTerm = frame->term->subTerms[0];
        stack.push(std::make_unique<ParseStackFrame>(subTerm, inputState, frame->productionStateRef));
        frame->subTermResult = nullptr;
        frame->step++;
    } else {
        // receive subTerms[0]
        if (frame->subTermResult->isMatch) {
            appendCapture(frame->productionStateRef->captures[frame->term->captureId], frame->subTermResult->box);
            stack.pop();
            stack.top()->subTermResult = TermResult::newSuccess();
        } else {
            auto result = std::move(frame->subTermResult);
            stack.pop();
            stack.top()->subTermResult = std::move(result);
        }
    }
}

static void pumpParseCut(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>& stack,
    InputState& inputState) {
    // single step
    frame->productionStateRef->cutHasBeenHit = true;
    stack.pop();
    stack.top()->subTermResult = TermResult::newSuccess();
}

static void pumpParseNonTerminal(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>& stack,
    InputState& inputState) {
    // step 0: start sub-production
    // step 1: receive sub-production, return

    if (frame->step == 0) {
        // start sub-production
        stack.push(std::make_unique<ParseStackFrame>(
            *frame->term->production, inputState, std::make_unique<ProductionState>(peekToken(inputState))));
        frame->subTermResult = nullptr;
        frame->step++;
    } else {
        // receive sub-production
        auto result = std::move(frame->subTermResult);

        // return
        stack.pop();
        stack.top()->subTermResult = std::move(result);
    }
}

static void pumpParseOptional(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>& stack,
    InputState& inputState) {
    // step 0: start subTerms[0]
    // step 1: receive subTerms[0], start subTerms[1]
    // ...
    // step N: receive subTerms[n-1], return

    if (frame->step > 0) {
        // receive subTerms[step-1]
        if (!frame->subTermResult->isMatch) {
            auto result = std::move(frame->subTermResult);
            frame->checkpoint->revert(inputState, *frame->productionStateRef);
            stack.pop();
            // if it didn't match (but isn't an error), that's fine because this was optional
            stack.top()->subTermResult = result->isError ? std::move(result) : TermResult::newSuccess();
            return;
        }
    }

    if (frame->step < frame->term->subTerms.size()) {
        // start subTerms[step]
        const auto& subTerm = frame->term->subTerms[frame->step];
        stack.push(std::make_unique<ParseStackFrame>(subTerm, inputState, frame->productionStateRef));
        frame->subTermResult = nullptr;
        frame->step++;
    } else {
        // return
        stack.pop();
        stack.top()->subTermResult = TermResult::newSuccess();
    }
}

static void pumpParseOr(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>& stack,
    InputState& inputState) {
    // step 0: start subTerms[0]
    // step 1: receive subTerms[0], start subTerms[1]
    // ...
    // step N: receive subTerms[n-1], return

    if (frame->step > 0) {
        // receive subTerms[step-1]
        if (frame->subTermResult->isError || frame->subTermResult->isMatch) {
            auto result = std::move(frame->subTermResult);
            stack.pop();
            stack.top()->subTermResult = std::move(result);
            return;
        } else {
            auto result = std::move(frame->subTermResult);
            frame->checkpoint->revert(inputState, *frame->productionStateRef);
            // it's fine, try the next one
        }
    }

    if (frame->step < frame->term->subTerms.size()) {
        // start subTerms[step]
        const auto& subTerm = frame->term->subTerms[frame->step];
        stack.push(std::make_unique<ParseStackFrame>(subTerm, inputState, frame->productionStateRef));
        frame->subTermResult = nullptr;
        frame->step++;
    } else {
        // if we made it this far, then none matched. return
        stack.pop();
        stack.top()->subTermResult =
            TermResult::newMismatchOrError(*frame->productionStateRef, *frame->term, inputState.currentToken());
    }
}

static void pumpParseTerminal(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>& stack,
    InputState& inputState) {
    // single step
    auto terminalToken = peekToken(inputState);
    if (frame->term->tokenKind == terminalToken.type) {
        acceptToken(inputState);
        stack.pop();
        stack.top()->subTermResult = TermResult::newSuccess(tokenBox(terminalToken));
    } else {
        frame->checkpoint->revert(inputState, *frame->productionStateRef);
        auto& term = *frame->term;
        stack.pop();
        stack.top()->subTermResult =
            TermResult::newMismatchOrError(*frame->productionStateRef, term, inputState.currentToken());
    }
}

static void pumpParseZeroOrMore(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>& stack,
    InputState& inputState) {
    // step 0: start subTerms[0]
    // step N: receive subTerms[0], start subTerms[0] again or return.

    if (frame->step > 0) {
        // receive subTerms[0]
        frame->productionStateRef->cutHasBeenHit = frame->previousCutStatus;
        if (!frame->subTermResult->isMatch) {
            frame->elementCheckpoint->revert(inputState, *frame->productionStateRef);
            if (frame->subTermResult->isError) {
                // it matched up to the cut, so this is an error
                stack.pop();
                stack.top()->subTermResult = std::move(frame->subTermResult);
            } else {
                // it's ok, end of the list
                stack.pop();
                stack.top()->subTermResult = TermResult::newSuccess();
            }
            return;
        }
    }

    // start subTerms[0]
    frame->elementCheckpoint = std::make_unique<Checkpoint>(inputState, *frame->productionStateRef);
    frame->previousCutStatus = frame->productionStateRef->cutHasBeenHit;
    frame->productionStateRef->cutHasBeenHit = false;
    stack.push(std::make_unique<ParseStackFrame>(frame->term->subTerms[0], inputState, frame->productionStateRef));
    frame->subTermResult = nullptr;
    frame->step++;
}

static void pumpParseProduction(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>& stack,
    InputState& inputState) {
    // step 0: start listTerm
    // step 1: receive listTerm

    if (frame->step == 0) {
        // start listTerm
        stack.push(
            std::make_unique<ParseStackFrame>(frame->production->listTerm, inputState, frame->productionStateRef));
        frame->subTermResult = nullptr;
        frame->step++;
    } else {
        // receive listTerm
        if (frame->subTermResult->isMatch) {
            auto termResult = TermResult::newSuccess(
                frame->production->parse(frame->productionStateRef->captures, frame->productionStateRef->firstToken));
            stack.pop();
            stack.top()->subTermResult = std::move(termResult);
        } else {
            auto result = std::move(frame->subTermResult);
            stack.pop();
            stack.top()->subTermResult = std::move(result);
        }
    }
}

static void pumpParse(std::stack<std::unique_ptr<ParseStackFrame>>& stack, InputState& inputState) {
    auto frame = stack.top().get();

#ifdef DUMP_PARSE
    if (frame->subTermResult) {
        if (frame->subTermResult->isMatch) {
            std::cerr << " match";
        } else if (frame->subTermResult->isError) {
            std::cerr << " error";
        } else {
            std::cerr << " no";
        }
    }
    std::cerr << std::endl;

    for (size_t i = 0; i < stack.size(); i++) {
        std::cerr << ' ';
    }
    std::cerr << frame->step << ' ';
    if (frame->production) {
        std::cerr << frame->production->name;
    } else {
        std::cerr << NAMEOF_ENUM(frame->term->type);
        if (frame->term->type == TermType::kTerminal) {
            std::cerr << ' ' << NAMEOF_ENUM(frame->term->tokenKind);
        }
    }
#endif

    if (frame->production) {
        pumpParseProduction(frame, stack, inputState);
    } else {
        switch (frame->term->type) {
            case TermType::kAnd:
                pumpParseAnd(frame, stack, inputState);
                break;

            case TermType::kCapture:
                pumpParseCapture(frame, stack, inputState);
                break;

            case TermType::kCut:
                pumpParseCut(frame, stack, inputState);
                break;

            case TermType::kNonTerminal:
                pumpParseNonTerminal(frame, stack, inputState);
                break;

            case TermType::kOptional:
                pumpParseOptional(frame, stack, inputState);
                break;

            case TermType::kOr:
                pumpParseOr(frame, stack, inputState);
                break;

            case TermType::kTerminal:
                pumpParseTerminal(frame, stack, inputState);
                break;

            case TermType::kZeroOrMore:
                pumpParseZeroOrMore(frame, stack, inputState);
                break;

            default:
                assert(false);
                break;
        }
    }
}

static ParserResult parseRootProduction(const Production& production, const std::vector<basic::Token>& tokens) {
    auto inputState = InputState(tokens);
    auto stack = std::stack<std::unique_ptr<ParseStackFrame>>();

    stack.push(std::make_unique<ParseStackFrame>());

    stack.push(std::make_unique<ParseStackFrame>(
        production, inputState, std::make_unique<ProductionState>(peekToken(inputState))));

    while (stack.size() > 1) {
        pumpParse(stack, inputState);
    }

#ifdef DUMP_PARSE
    std::cerr << std::endl;
#endif

    auto& result = stack.top()->subTermResult;
    if (result->isMatch) {
        if (inputState.tokenIndex < inputState.tokens.size()) {
            // the code is good but there is unparsed junk left over
            return ParserResult("This token was unexpected.", inputState.tokens[inputState.tokenIndex]);
        } else {
            return ParserResult(captureSingleNode<Node>(std::move(result->box)));
        }
    } else {
        return ParserResult("This token was unexpected.", inputState.tokens[inputState.tokenIndex]);
    }
}

ParserResult Parser::parseProgram(const std::vector<basic::Token>& tokens) {
    return parseRootProduction(*_productionCollection->programProduction, tokens);
}

ParserResult Parser::parseMember(const std::vector<basic::Token>& tokens) {
    return parseRootProduction(*_productionCollection->memberProduction, tokens);
}

}  // namespace compiler
