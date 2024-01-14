// uncomment to dump parse tree to std::cerr
// #define DUMP_PARSE

// uncomment to dump final AST to std::cerr
// #define DUMP_AST

#include "compiler/parse.h"
#include "shared/decimal.h"
#include "shared/cast.h"

using shared::dynamic_cast_borrow;
using shared::dynamic_cast_move;
using shared::parseDecimalString;

namespace compiler {

const int kNumCaptures = 5;

enum class TermType { kAnd, kCapture, kCut, kNonTerminal, kOptional, kOr, kTerminal, kZeroOrMore };

class Production;

class Term {
   public:
    TermType type;
    std::vector<Term> subTerms;
    const Production* production{ nullptr };   // when type = kNonTerminal
    TokenKind tokenKind{ TokenKind::kError };  // when type = kTerminal
    int captureId{ -1 };                       // when type = kCapture
    explicit Term(TermType type) : type(type) {}
};

enum class BoxType { kNode, kToken };

class Box {
   public:
    BoxType type;
    explicit Box(BoxType type) : type(type) {}
    Box(const Box&) = delete;
    Box& operator=(const Box&) = delete;
    Box(Box&&) = delete;
    Box& operator=(Box&&) = delete;
    virtual ~Box() = 0;
    virtual size_t count() = 0;
    virtual void removeAt(ptrdiff_t index) = 0;
};

using CaptureArray = std::array<std::unique_ptr<Box>, kNumCaptures>;

Box::~Box() = default;

class NodeBox : public Box {
   public:
    std::vector<std::unique_ptr<Node>> values;

    NodeBox() : Box(BoxType::kNode) {}
    size_t count() override { return values.size(); }
    void removeAt(ptrdiff_t index) override { values.erase(values.begin() + index); }
    void append(NodeBox* other) {
        for (auto& node : other->values) {
            values.push_back(std::move(node));
        }
    }
    std::unique_ptr<Node> value() {
        if (values.empty()) {
            return nullptr;
        }
        return std::move(values.at(0));
    }
};

class TokenBox : public Box {
   public:
    std::vector<Token> values;

    TokenBox() : Box(BoxType::kToken) {}
    size_t count() override { return values.size(); }
    void removeAt(ptrdiff_t index) override { values.erase(values.begin() + index); }
    void append(const TokenBox& other) {
        for (const auto& node : other.values) {
            values.push_back(node);
        }
    }
    Token value() const {
        if (values.empty()) {
            return {};
        }
        return values.at(0);
    }
};

class Production {
   public:
    std::string_view name;  // to ease debugging; not used for any logic
    Term listTerm;
    Production(std::string_view name, std::initializer_list<Term> terms);
    Production(const Production&) = delete;
    Production& operator=(const Production&) = delete;
    Production(Production&&) = delete;
    Production& operator=(Production&&) = delete;
    virtual ~Production() = default;
    virtual std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const = 0;
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
    for (const auto& subTerm : terms) {
        t.subTerms.push_back(subTerm);
    }
    return t;
}

static Term list(std::initializer_list<Term> terms) {
    auto t = Term(TermType::kAnd);
    for (const auto& subTerm : terms) {
        t.subTerms.push_back(subTerm);
    }
    return t;
}

static Term oneOf(std::initializer_list<Term> terms) {
    auto t = Term(TermType::kOr);
    for (const auto& subTerm : terms) {
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
static std::vector<std::unique_ptr<T>> captureNodeArray(std::unique_ptr<Box> box) {
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
    assert(box != nullptr);
    assert(box->type == BoxType::kNode);
    auto nodeBox = dynamic_cast_move<NodeBox>(std::move(box));
    return dynamic_cast_move<T>(nodeBox->value());
}

static bool hasCapture(const std::unique_ptr<Box>& box) {
    return box && box->count() > 0;
}

template <typename T>
static std::unique_ptr<T> captureSingleNodeOrNull(std::unique_ptr<Box> box) {
    if (hasCapture(box)) {
        return captureSingleNode<T>(std::move(box));
    }
    return {};
}

static Token captureToken(std::unique_ptr<Box> box) {
    assert(box->type == BoxType::kToken);
    auto tokenBox = dynamic_cast_move<TokenBox>(std::move(box));
    return tokenBox->value();
}

static Token captureTokenNoMove(const Box* box) {
    assert(box->type == BoxType::kToken);
    const auto* tokenBox = dynamic_cast<const TokenBox*>(box);
    return tokenBox->value();
}

static std::string captureTokenText(std::unique_ptr<Box> box) {
    return captureToken(std::move(box)).text;
}

static TokenKind captureTokenKind(std::unique_ptr<Box> box) {
    return captureToken(std::move(box)).type;
}

template <typename T, typename... Args>
static std::unique_ptr<NodeBox> nodeBox(Args&&... args) {
    auto box = std::make_unique<NodeBox>();
    box->values.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    return box;
}

static std::unique_ptr<TokenBox> tokenBox(const Token& token) {
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
        case TokenKind::kCaret:
            return BinaryOperator::kPower;
        default:
            assert(false);
            return {};
    }
}

static std::unique_ptr<Box> parseBinaryExpressionSuffix(CaptureArray* captures, const Token& firstToken) {
    return nodeBox<BinaryExpressionSuffixNode>(
        TokenKindToBinaryOperator(captureTokenKind(std::move(captures->at(0)))),
        captureSingleNode<ExpressionNode>(std::move(captures->at(1))), firstToken);
}

static std::unique_ptr<Box> parseBinaryExpression(CaptureArray* captures) {
    if (hasCapture(captures->at(1))) {
        auto suffixes = captureNodeArray<BinaryExpressionSuffixNode>(std::move(captures->at(1)));
        auto token = suffixes.at(0)->token;
        return nodeBox<BinaryExpressionNode>(
            captureSingleNode<ExpressionNode>(std::move(captures->at(0))), std::move(suffixes), token);
    }
    return std::move(captures->at(0));
}

//
// productions
//

class BodyProduction : public Production {
   public:
    explicit BodyProduction(const Production* statement)
        : Production(
              NAMEOF_TYPE(BodyProduction),
              {
                  zeroOrMore({
                      capture(0, prod(statement)),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        auto statements = captureNodeArray<StatementNode>(std::move(captures->at(0)));
        return nodeBox<BodyNode>(std::move(statements), firstToken);
    }
};

class ArgumentListProduction : public Production {
   public:
    explicit ArgumentListProduction(const Production* expression)
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return std::move(captures->at(0));
    }
};

class ParameterProduction : public Production {
   public:
    explicit ParameterProduction(const Production* type)
        : Production(
              NAMEOF_TYPE(ParameterProduction),
              {
                  capture(0, term(TokenKind::kIdentifier)),
                  cut(),
                  term(TokenKind::kAs),
                  capture(1, prod(type)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        auto type = captureSingleNode<TypeNode>(std::move(captures->at(1)));
        return nodeBox<ParameterNode>(captureTokenText(std::move(captures->at(0))), std::move(type), firstToken);
    }
};

class ParameterListProduction : public Production {
   public:
    explicit ParameterListProduction(const Production* parameter)
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return std::move(captures->at(0));
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<TypeNode>(Kind::kRecord, firstToken, captureTokenText(std::move(captures->at(0))));
    }
};

class TypeWithParenthesesProduction : public Production {
   public:
    explicit TypeWithParenthesesProduction(const Production* type)
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return std::move(captures->at(0));
    }
};

class OptionalTypeProduction : public Production {
   public:
    explicit OptionalTypeProduction(const Production* typeWithParentheses)
        : Production(
              NAMEOF_TYPE(OptionalTypeProduction),
              {
                  term(TokenKind::kOptional),
                  cut(),
                  capture(0, prod(typeWithParentheses)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<TypeNode>(Kind::kOptional, firstToken, captureSingleNode<TypeNode>(std::move(captures->at(0))));
    }
};

class MapTypeProduction : public Production {
   public:
    explicit MapTypeProduction(const Production* typeWithParentheses)
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<TypeNode>(
            Kind::kMap, firstToken, captureSingleNode<TypeNode>(std::move(captures->at(0))),
            captureSingleNode<TypeNode>(std::move(captures->at(1))));
    }
};

class SetTypeProduction : public Production {
   public:
    explicit SetTypeProduction(const Production* typeWithParentheses)
        : Production(
              NAMEOF_TYPE(SetTypeProduction),
              {
                  term(TokenKind::kSet),
                  cut(),
                  term(TokenKind::kOf),
                  capture(0, prod(typeWithParentheses)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<TypeNode>(Kind::kSet, firstToken, captureSingleNode<TypeNode>(std::move(captures->at(0))));
    }
};

class ListTypeProduction : public Production {
   public:
    explicit ListTypeProduction(const Production* typeWithParentheses)
        : Production(
              NAMEOF_TYPE(ListTypeProduction),
              {
                  term(TokenKind::kList),
                  cut(),
                  term(TokenKind::kOf),
                  capture(0, prod(typeWithParentheses)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<TypeNode>(Kind::kList, firstToken, captureSingleNode<TypeNode>(std::move(captures->at(0))));
    }
};

class RecordTypeProduction : public Production {
   public:
    explicit RecordTypeProduction(const Production* parameterList)
        : Production(
              NAMEOF_TYPE(RecordTypeProduction),
              {
                  term(TokenKind::kRecord),
                  cut(),
                  term(TokenKind::kLeftParenthesis),
                  capture(0, prod(parameterList)),
                  term(TokenKind::kRightParenthesis),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        std::vector<boost::local_shared_ptr<ParameterNode>> fields;
        for (auto& parameter : captureNodeArray<ParameterNode>(std::move(captures->at(0)))) {
            fields.push_back(
                boost::make_local_shared<ParameterNode>(parameter->name, std::move(parameter->type), firstToken));
        }
        return nodeBox<TypeNode>(Kind::kRecord, firstToken, std::move(fields));
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
                          term(TokenKind::kString),
                          term(TokenKind::kDate),
                          term(TokenKind::kDateTime),
                          term(TokenKind::kDateTimeOffset),
                          term(TokenKind::kTimeSpan),
                          term(TokenKind::kTimeZone),
                          term(TokenKind::kForm),
                          term(TokenKind::kControl),
                      })),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        auto k = Kind::kBoolean;
        switch (captureTokenKind(std::move(captures->at(0)))) {
            case TokenKind::kBoolean:
                k = Kind::kBoolean;
                break;
            case TokenKind::kNumber:
                k = Kind::kNumber;
                break;
            case TokenKind::kString:
                k = Kind::kString;
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
            case TokenKind::kTimeZone:
                k = Kind::kTimeZone;
                break;
            case TokenKind::kTimeSpan:
                k = Kind::kTimeSpan;
                break;
            case TokenKind::kForm:
                k = Kind::kForm;
                break;
            case TokenKind::kControl:
                k = Kind::kControl;
                break;
            default:
                assert(false);
                break;
        }
        return nodeBox<TypeNode>(k, firstToken);
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
        const Production* setType,
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
                    prod(setType),
                    prod(optionalType),
                    prod(namedType),
                })),
        });
    }

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return std::move(captures->at(0));
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
                          term(TokenKind::kTrue),
                          term(TokenKind::kFalse),
                          term(TokenKind::kNumberLiteral),
                          term(TokenKind::kStringLiteral),
                      })),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        auto token = captureToken(std::move(captures->at(0)));
        switch (token.type) {
            case TokenKind::kTrue:
                return nodeBox<LiteralBooleanExpressionNode>(true, firstToken);

            case TokenKind::kFalse:
                return nodeBox<LiteralBooleanExpressionNode>(false, firstToken);

            case TokenKind::kNumberLiteral:
                return nodeBox<LiteralNumberExpressionNode>(parseDecimalString(token.text), firstToken);

            case TokenKind::kStringLiteral: {
                auto insidePart = token.text.substr(1, token.text.length() - 2);
                auto doubleQuote = std::string("\"\"");
                auto singleQuote = std::string("\"");
                auto pos = insidePart.find(doubleQuote);
                while (pos != std::string::npos) {
                    insidePart.replace(pos, doubleQuote.size(), singleQuote);
                    pos = insidePart.find(doubleQuote, pos + singleQuote.length());
                }
                return nodeBox<LiteralStringExpressionNode>(insidePart, firstToken);
            }

            default:
                assert(false);
                return {};
        }
    }
};

class LiteralRecordFieldProduction : public Production {
   public:
    explicit LiteralRecordFieldProduction(const Production* expression)
        : Production(
              NAMEOF_TYPE(LiteralRecordFieldProduction),
              {
                  capture(0, term(TokenKind::kIdentifier)),
                  cut(),
                  term(TokenKind::kColon),
                  capture(1, prod(expression)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<LiteralRecordFieldNode>(
            captureTokenText(std::move(captures->at(0))), captureSingleNode<ExpressionNode>(std::move(captures->at(1))),
            firstToken);
    }
};

class LiteralRecordFieldListProduction : public Production {
   public:
    explicit LiteralRecordFieldListProduction(const Production* literalRecordField)
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return std::move(captures->at(0));
    }
};

class LiteralRecordTermProduction : public Production {
   public:
    explicit LiteralRecordTermProduction(const Production* literalRecordFieldList)
        : Production(
              NAMEOF_TYPE(LiteralRecordTermProduction),
              {
                  term(TokenKind::kLeftBrace),
                  cut(),
                  capture(0, prod(literalRecordFieldList)),
                  term(TokenKind::kRightBrace),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<LiteralRecordExpressionNode>(
            captureNodeArray<LiteralRecordFieldNode>(std::move(captures->at(0))), firstToken);
    }
};

class LiteralNoTermProduction : public Production {
   public:
    explicit LiteralNoTermProduction(const Production* type)
        : Production(
              NAMEOF_TYPE(LiteralNoTermProduction),
              {
                  term(TokenKind::kNo),
                  cut(),
                  capture(0, prod(type)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<LiteralNoExpressionNode>(captureSingleNode<TypeNode>(std::move(captures->at(0))), firstToken);
    }
};

class LiteralArrayTermProduction : public Production {
   public:
    explicit LiteralArrayTermProduction(const Production* argumentList)
        : Production(
              NAMEOF_TYPE(LiteralArrayTermProduction),
              {
                  term(TokenKind::kLeftBracket),
                  cut(),
                  capture(0, prod(argumentList)),
                  term(TokenKind::kRightBracket),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<LiteralArrayExpressionNode>(
            captureNodeArray<ExpressionNode>(std::move(captures->at(0))), firstToken);
    }
};

class ParenthesesTermProduction : public Production {
   public:
    explicit ParenthesesTermProduction(const Production* expression)
        : Production(
              NAMEOF_TYPE(ParenthesesTermProduction),
              {
                  term(TokenKind::kLeftParenthesis),
                  cut(),
                  capture(0, prod(expression)),
                  term(TokenKind::kRightParenthesis),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return std::move(captures->at(0));
    }
};

class ExpressionTermProduction : public Production {
   public:
    ExpressionTermProduction(
        const Production* literalValue,
        const Production* parenthesesTerm,
        const Production* literalArrayTerm,
        const Production* literalRecordTerm,
        const Production* literalNoTerm)
        : Production(
              NAMEOF_TYPE(ExpressionTermProduction),
              {
                  oneOf({
                      capture(0, prod(literalValue)),
                      capture(0, prod(parenthesesTerm)),
                      capture(0, prod(literalArrayTerm)),
                      capture(0, prod(literalRecordTerm)),
                      capture(0, prod(literalNoTerm)),
                      capture(1, term(TokenKind::kIdentifier)),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        if (hasCapture(captures->at(0))) {
            return std::move(captures->at(0));
        }
        if (hasCapture(captures->at(1))) {
            return nodeBox<SymbolReferenceExpressionNode>(captureTokenText(std::move(captures->at(1))), firstToken);
        }
        assert(false);
        return {};
    }
};

class DottedExpressionSuffixProduction : public Production {
   public:
    explicit DottedExpressionSuffixProduction(const Production* expression)
        : Production(
              NAMEOF_TYPE(DottedExpressionSuffixProduction),
              {
                  oneOf({
                      // field access
                      list({
                          term(TokenKind::kDot),
                          cut(),
                          capture(0, term(TokenKind::kIdentifier)),
                      }),
                      // collection index or function call
                      list({
                          term(TokenKind::kLeftParenthesis),
                          cut(),
                          optional({
                              capture(1, prod(expression)),
                              zeroOrMore({
                                  term(TokenKind::kComma),
                                  capture(1, prod(expression)),
                              }),
                          }),
                          term(TokenKind::kRightParenthesis),
                      }),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        if (hasCapture(captures->at(0))) {
            // field access
            return nodeBox<DottedExpressionSuffixNode>(captureTokenText(std::move(captures->at(0))), firstToken);
        }

        // collection index or function call
        return nodeBox<DottedExpressionSuffixNode>(
            captureNodeArray<ExpressionNode>(std::move(captures->at(1))), firstToken);
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        auto dottedSuffixes = captureNodeArray<DottedExpressionSuffixNode>(std::move(captures->at(1)));
        if (dottedSuffixes.empty()) {
            return std::move(captures->at(0));
        }
        auto token = dottedSuffixes.at(0)->token;
        return nodeBox<DottedExpressionNode>(
            captureSingleNode<ExpressionNode>(std::move(captures->at(0))), std::move(dottedSuffixes), token);
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        if (hasCapture(captures->at(1))) {
            return nodeBox<ConvertExpressionNode>(
                captureSingleNode<ExpressionNode>(std::move(captures->at(0))),
                captureSingleNode<TypeNode>(std::move(captures->at(1))), firstToken);
        }
        return std::move(captures->at(0));
    }
};

class UnaryExpressionProduction : public Production {
   public:
    explicit UnaryExpressionProduction(const Production* convertExpression)
        : Production(
              NAMEOF_TYPE(UnaryExpressionProduction),
              {
                  optional({
                      capture(0, term(TokenKind::kNot)),
                  }),
                  capture(1, prod(convertExpression)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        if (hasCapture(captures->at(0))) {
            return nodeBox<NotExpressionNode>(
                captureSingleNode<ExpressionNode>(std::move(captures->at(1))), firstToken);
        }
        return std::move(captures->at(1));
    }
};

class ExponentExpressionSuffixProduction : public Production {
   public:
    explicit ExponentExpressionSuffixProduction(const Production* unaryExpression)
        : Production(
              NAMEOF_TYPE(ExponentExpressionSuffixProduction),
              {
                  capture(0, term(TokenKind::kCaret)),
                  capture(1, prod(unaryExpression)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return parseBinaryExpressionSuffix(captures, firstToken);
    }
};

class ExponentExpressionProduction : public Production {
   public:
    ExponentExpressionProduction(const Production* unaryExpression, const Production* exponentExpressionSuffix)
        : Production(
              NAMEOF_TYPE(ExponentExpressionProduction),
              {
                  capture(0, prod(unaryExpression)),
                  zeroOrMore({
                      capture(1, prod(exponentExpressionSuffix)),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return parseBinaryExpression(captures);
    }
};

class MultiplyExpressionSuffixProduction : public Production {
   public:
    explicit MultiplyExpressionSuffixProduction(const Production* exponentExpression)
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
                  capture(1, prod(exponentExpression)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return parseBinaryExpressionSuffix(captures, firstToken);
    }
};

class MultiplyExpressionProduction : public Production {
   public:
    MultiplyExpressionProduction(const Production* exponentExpression, const Production* multiplyExpressionSuffix)
        : Production(
              NAMEOF_TYPE(MultiplyExpressionProduction),
              {
                  capture(0, prod(exponentExpression)),
                  zeroOrMore({
                      capture(1, prod(multiplyExpressionSuffix)),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return parseBinaryExpression(captures);
    }
};

class AddExpressionSuffixProduction : public Production {
   public:
    explicit AddExpressionSuffixProduction(const Production* multiplyExpression)
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return parseBinaryExpression(captures);
    }
};

class InequalityExpressionSuffixProduction : public Production {
   public:
    explicit InequalityExpressionSuffixProduction(const Production* addExpression)
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return parseBinaryExpression(captures);
    }
};

class EqualityExpressionSuffixProduction : public Production {
   public:
    explicit EqualityExpressionSuffixProduction(const Production* inequalityExpression)
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return parseBinaryExpression(captures);
    }
};

class AndExpressionSuffixProduction : public Production {
   public:
    explicit AndExpressionSuffixProduction(const Production* equalityExpression)
        : Production(
              NAMEOF_TYPE(AndExpressionSuffixProduction),
              {
                  capture(0, term(TokenKind::kAnd)),
                  capture(1, prod(equalityExpression)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return parseBinaryExpression(captures);
    }
};

class OrExpressionSuffixProduction : public Production {
   public:
    explicit OrExpressionSuffixProduction(const Production* andExpression)
        : Production(
              NAMEOF_TYPE(OrExpressionSuffixProduction),
              {
                  capture(0, term(TokenKind::kOr)),
                  capture(1, prod(andExpression)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return parseBinaryExpression(captures);
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return std::move(captures->at(0));
    }
};

class CatchBlockProduction : public Production {
   public:
    explicit CatchBlockProduction(const Production* body)
        : Production(
              NAMEOF_TYPE(CatchBlockProduction),
              {
                  term(TokenKind::kCatch),
                  cut(),
                  term(TokenKind::kEndOfLine),
                  capture(0, prod(body)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return std::move(captures->at(0));
    }
};

class TryStatementProduction : public Production {
   public:
    TryStatementProduction(const Production* body, const Production* catchBlock)
        : Production(
              NAMEOF_TYPE(TryStatementProduction),
              {
                  term(TokenKind::kTry),
                  cut(),
                  term(TokenKind::kEndOfLine),
                  capture(0, prod(body)),
                  capture(1, prod(catchBlock)),
                  term(TokenKind::kEnd),
                  term(TokenKind::kTry),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<TryStatementNode>(
            captureSingleNode<BodyNode>(std::move(captures->at(0))),
            captureSingleNode<BodyNode>(std::move(captures->at(1))), firstToken);
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

    std::unique_ptr<Box> parse(CaptureArray* /*captures*/, const Token& firstToken) const override {
        return nodeBox<RethrowStatementNode>(firstToken);
    }
};

class ThrowStatementProduction : public Production {
   public:
    explicit ThrowStatementProduction(const Production* expression)
        : Production(
              NAMEOF_TYPE(ThrowStatementProduction),
              {
                  term(TokenKind::kThrow),
                  cut(),
                  capture(0, prod(expression)),
                  optional({
                      term(TokenKind::kComma),
                      capture(1, prod(expression)),
                  }),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        if (hasCapture(captures->at(1))) {
            return nodeBox<ThrowStatementNode>(
                captureSingleNode<ExpressionNode>(std::move(captures->at(1))),
                captureSingleNode<ExpressionNode>(std::move(captures->at(0))), firstToken);
        }
        return nodeBox<ThrowStatementNode>(
            captureSingleNode<ExpressionNode>(std::move(captures->at(0))), nullptr, firstToken);
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
                          term(TokenKind::kWhile),
                      })),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        LoopType scope{};
        switch (captureTokenKind(std::move(captures->at(0)))) {
            case TokenKind::kDo:
                scope = LoopType::kDo;
                break;
            case TokenKind::kFor:
                scope = LoopType::kFor;
                break;
            case TokenKind::kWhile:
                scope = LoopType::kWhile;
                break;
            default:
                assert(false);
                break;
        }
        return nodeBox<ExitStatementNode>(scope, firstToken);
    }
};

class ContinueStatementProduction : public Production {
   public:
    ContinueStatementProduction()
        : Production(
              NAMEOF_TYPE(ContinueStatementProduction),
              {
                  term(TokenKind::kContinue),
                  capture(
                      0,
                      oneOf({
                          term(TokenKind::kDo),
                          term(TokenKind::kFor),
                          term(TokenKind::kWhile),
                      })),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        LoopType scope{};
        switch (captureTokenKind(std::move(captures->at(0)))) {
            case TokenKind::kDo:
                scope = LoopType::kDo;
                break;
            case TokenKind::kFor:
                scope = LoopType::kFor;
                break;
            case TokenKind::kWhile:
                scope = LoopType::kWhile;
                break;
            default:
                assert(false);
                break;
        }
        return nodeBox<ContinueStatementNode>(scope, firstToken);
    }
};

class CallStatementProduction : public Production {
   public:
    explicit CallStatementProduction(const Production* argumentList)
        : Production(
              NAMEOF_TYPE(CallStatementProduction),
              {
                  capture(0, term(TokenKind::kIdentifier)),
                  cut(),
                  capture(1, prod(argumentList)),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<CallStatementNode>(
            captureTokenText(std::move(captures->at(0))), captureNodeArray<ExpressionNode>(std::move(captures->at(1))),
            firstToken);
    }
};

class ReturnStatementProduction : public Production {
   public:
    explicit ReturnStatementProduction(const Production* expression)
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<ReturnStatementNode>(
            captureSingleNodeOrNull<ExpressionNode>(std::move(captures->at(0))), firstToken);
    }
};

class YieldStatementProduction : public Production {
   public:
    explicit YieldStatementProduction(const Production* expression)
        : Production(
              NAMEOF_TYPE(YieldStatementProduction),
              {
                  term(TokenKind::kYield),
                  cut(),
                  capture(0, prod(expression)),
                  optional({
                      term(TokenKind::kTo),
                      capture(1, prod(expression)),
                  }),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<YieldStatementNode>(
            captureSingleNode<ExpressionNode>(std::move(captures->at(0))),
            captureSingleNodeOrNull<ExpressionNode>(std::move(captures->at(1))), firstToken);
    }
};

class ConstStatementProduction : public Production {
   public:
    explicit ConstStatementProduction(const Production* literalValue)
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        auto token = captureTokenNoMove(captures->at(0).get());
        return nodeBox<ConstStatementNode>(
            captureTokenText(std::move(captures->at(0))),
            captureSingleNode<ConstValueExpressionNode>(std::move(captures->at(1))), token);
    }
};

class AssignStatementProduction : public Production {
   public:
    AssignStatementProduction(const Production* expression, const Production* dottedExpression)
        : Production(
              NAMEOF_TYPE(AssignStatementProduction),
              {
                  capture(0, prod(dottedExpression)),
                  term(TokenKind::kEqualsSign),
                  capture(1, prod(expression)),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<AssignStatementNode>(
            captureSingleNode<ExpressionNode>(std::move(captures->at(0))),
            captureSingleNode<ExpressionNode>(std::move(captures->at(1))), firstToken);
    }
};

class DimStatementProduction : public Production {
   public:
    DimStatementProduction(const Production* type, const Production* expression)
        : Production(
              NAMEOF_TYPE(DimStatementProduction),
              {
                  term(TokenKind::kDim),  // no cut here because it could be "dim list"
                  optional({
                      capture(3, term(TokenKind::kShared)),
                  }),
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        auto shared = hasCapture(captures->at(3));
        if (hasCapture(captures->at(1))) {
            return nodeBox<DimStatementNode>(
                captureTokenText(std::move(captures->at(0))), captureSingleNode<TypeNode>(std::move(captures->at(1))),
                firstToken, shared);
        }
        return nodeBox<DimStatementNode>(
            captureTokenText(std::move(captures->at(0))), captureSingleNode<ExpressionNode>(std::move(captures->at(2))),
            firstToken, shared);
    }
};

class DimCollectionStatementProduction : public Production {
   public:
    explicit DimCollectionStatementProduction(const Production* body)
        : Production(
              NAMEOF_TYPE(DimCollectionStatementProduction),
              {
                  term(TokenKind::kDim),
                  capture(
                      0,
                      oneOf({
                          term(TokenKind::kList),
                          term(TokenKind::kMap),
                          term(TokenKind::kSet),
                      })),
                  capture(1, term(TokenKind::kIdentifier)),
                  term(TokenKind::kEndOfLine),
                  capture(2, prod(body)),
                  term(TokenKind::kEnd),
                  term(TokenKind::kDim),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        switch (captureTokenKind(std::move(captures->at(0)))) {
            case TokenKind::kList:
                return nodeBox<DimListStatementNode>(
                    captureTokenText(std::move(captures->at(1))),
                    captureSingleNode<BodyNode>(std::move(captures->at(2))), firstToken);
                break;
            case TokenKind::kMap:
                return nodeBox<DimMapStatementNode>(
                    captureTokenText(std::move(captures->at(1))),
                    captureSingleNode<BodyNode>(std::move(captures->at(2))), firstToken);
                break;
            case TokenKind::kSet:
                return nodeBox<DimSetStatementNode>(
                    captureTokenText(std::move(captures->at(1))),
                    captureSingleNode<BodyNode>(std::move(captures->at(2))), firstToken);
                break;
            default:
                assert(false);
                return {};
        }
    }
};

class CaseValueProduction : public Production {
   public:
    explicit CaseValueProduction(const Production* expression)
        : Production(
              NAMEOF_TYPE(CaseValueProduction),
              {
                  capture(0, prod(expression)),
                  optional({
                      term(TokenKind::kTo),
                      capture(1, prod(expression)),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<CaseValueNode>(
            captureSingleNode<ExpressionNode>(std::move(captures->at(0))),
            captureSingleNodeOrNull<ExpressionNode>(std::move(captures->at(1))), firstToken);
    }
};

class CaseValueListProduction : public Production {
   public:
    explicit CaseValueListProduction(const Production* caseValue)
        : Production(
              NAMEOF_TYPE(CaseValueListProduction),
              {
                  capture(0, prod(caseValue)),
                  zeroOrMore({
                      term(TokenKind::kComma),
                      capture(0, prod(caseValue)),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return std::move(captures->at(0));
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<CaseNode>(
            captureNodeArray<CaseValueNode>(std::move(captures->at(0))),
            captureSingleNode<BodyNode>(std::move(captures->at(1))), firstToken);
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<SelectCaseStatementNode>(
            captureSingleNode<ExpressionNode>(std::move(captures->at(0))),
            captureNodeArray<CaseNode>(std::move(captures->at(1))), firstToken);
    }
};

class DoStatementProduction : public Production {
   public:
    DoStatementProduction(const Production* expression, const Production* body)
        : Production(
              NAMEOF_TYPE(DoStatementProduction),
              {
                  term(TokenKind::kDo),
                  cut(),
                  term(TokenKind::kEndOfLine),
                  capture(1, prod(body)),
                  term(TokenKind::kLoop),
                  term(TokenKind::kWhile),
                  capture(2, prod(expression)),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<DoStatementNode>(
            captureSingleNode<ExpressionNode>(std::move(captures->at(2))),
            captureSingleNode<BodyNode>(std::move(captures->at(1))), firstToken);
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<ElseIfNode>(
            captureSingleNode<ExpressionNode>(std::move(captures->at(0))),
            captureSingleNode<BodyNode>(std::move(captures->at(1))), firstToken);
    }
};

class IfStatementProduction : public Production {
   public:
    IfStatementProduction(
        const Production* expression,
        const Production* body,
        const Production* elseIf,
        const Production* statement)
        : Production(
              NAMEOF_TYPE(IfStatementProduction),
              {
                  term(TokenKind::kIf),
                  cut(),
                  capture(0, prod(expression)),
                  term(TokenKind::kThen),
                  oneOf({
                      list({
                          // Multi-line If.
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
                      }),
                      list({
                          // Single-line If.
                          capture(4, prod(statement)),
                      }),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        auto body = captureSingleNodeOrNull<BodyNode>(std::move(captures->at(1)));

        if (body == nullptr) {
            // Single-line If. Create a BodyNode containing the single statement.
            std::vector<std::unique_ptr<StatementNode>> statements{};
            auto statement = captureSingleNode<StatementNode>(std::move(captures->at(4)));
            statements.push_back(std::move(statement));
            body = std::make_unique<BodyNode>(std::move(statements), firstToken);

            return nodeBox<IfStatementNode>(
                captureSingleNode<ExpressionNode>(std::move(captures->at(0))), std::move(body),
                std::vector<std::unique_ptr<ElseIfNode>>{}, nullptr, firstToken);
        }

        // Multi-line If.
        return nodeBox<IfStatementNode>(
            captureSingleNode<ExpressionNode>(std::move(captures->at(0))), std::move(body),
            captureNodeArray<ElseIfNode>(std::move(captures->at(2))),
            captureSingleNodeOrNull<BodyNode>(std::move(captures->at(3))), firstToken);
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<WhileStatementNode>(
            captureSingleNode<ExpressionNode>(std::move(captures->at(0))),
            captureSingleNode<BodyNode>(std::move(captures->at(1))), firstToken);
    }
};

class ForEachStatementProduction : public Production {
   public:
    ForEachStatementProduction(const Production* expression, const Production* body)
        : Production(
              NAMEOF_TYPE(ForEachStatementProduction),
              {
                  term(TokenKind::kFor),
                  term(TokenKind::kEach),
                  cut(),
                  capture(0, term(TokenKind::kIdentifier)),
                  term(TokenKind::kIn),
                  capture(1, prod(expression)),
                  term(TokenKind::kEndOfLine),
                  capture(2, prod(body)),
                  term(TokenKind::kNext),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<ForEachStatementNode>(
            captureTokenText(std::move(captures->at(0))), captureSingleNode<ExpressionNode>(std::move(captures->at(1))),
            captureSingleNode<BodyNode>(std::move(captures->at(2))), firstToken);
    }
};

class ForStepProduction : public Production {
   public:
    explicit ForStepProduction(const Production* expression)
        : Production(
              NAMEOF_TYPE(ForStepProduction),
              {
                  term(TokenKind::kStep),
                  cut(),
                  capture(0, prod(expression)),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return std::move(captures->at(0));
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<ForStatementNode>(
            captureTokenText(std::move(captures->at(0))), captureSingleNode<ExpressionNode>(std::move(captures->at(1))),
            captureSingleNode<ExpressionNode>(std::move(captures->at(2))),
            captureSingleNodeOrNull<ExpressionNode>(std::move(captures->at(3))),
            captureSingleNode<BodyNode>(std::move(captures->at(4))), firstToken);
    }
};

class PrintStatementProduction : public Production {
   public:
    explicit PrintStatementProduction(const Production* expression)
        : Production(
              NAMEOF_TYPE(PrintStatementProduction),
              {
                  term(TokenKind::kPrint),
                  cut(),
                  capture(1, prod(expression)),
                  zeroOrMore({
                      term(TokenKind::kSemicolon),
                      capture(1, prod(expression)),
                  }),
                  optional({
                      capture(2, term(TokenKind::kSemicolon)),
                  }),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        auto expressions = captureNodeArray<ExpressionNode>(std::move(captures->at(1)));
        auto trailingSemicolon = hasCapture(captures->at(2));
        return nodeBox<PrintStatementNode>(std::move(expressions), trailingSemicolon, firstToken);
    }
};

class InputStatementProduction : public Production {
   public:
    explicit InputStatementProduction(const Production* expression)
        : Production(
              NAMEOF_TYPE(InputStatementProduction),
              {
                  term(TokenKind::kInput),
                  cut(),
                  capture(1, prod(expression)),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<InputStatementNode>(captureSingleNode<ExpressionNode>(std::move(captures->at(1))), firstToken);
    }
};

class OnStatementProduction : public Production {
   public:
    explicit OnStatementProduction(const Production* symbolReferenceExpression)
        : Production(
              NAMEOF_TYPE(OnStatementProduction),
              {
                  term(TokenKind::kOn),
                  cut(),
                  capture(1, prod(symbolReferenceExpression)),
                  capture(2, term(TokenKind::kIdentifier)),
                  term(TokenKind::kCall),
                  capture(3, term(TokenKind::kIdentifier)),
                  term(TokenKind::kEndOfLine),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<OnStatementNode>(
            captureSingleNode<SymbolReferenceExpressionNode>(std::move(captures->at(1))),
            captureTokenText(std::move(captures->at(2))), captureTokenText(std::move(captures->at(3))), firstToken);
    }
};

class CommandStatementProduction : public Production {
   public:
    CommandStatementProduction(
        const Production* assignStatement,
        const Production* yieldStatement,
        const Production* returnStatement,
        const Production* callStatement,
        const Production* continueStatement,
        const Production* exitStatement,
        const Production* throwStatement,
        const Production* rethrowStatement,
        const Production* printStatement,
        const Production* inputStatement)
        : Production(
              NAMEOF_TYPE(CommandStatementProduction),
              {
                  capture(
                      0,
                      oneOf({
                          prod(assignStatement),
                          prod(yieldStatement),
                          prod(returnStatement),
                          prod(callStatement),
                          prod(continueStatement),
                          prod(exitStatement),
                          prod(throwStatement),
                          prod(rethrowStatement),
                          prod(printStatement),
                          prod(inputStatement),
                      })),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return std::move(captures->at(0));
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
        const Production* selectCaseStatement,
        const Production* tryStatement,
        const Production* dimStatement,
        const Production* dimCollectionStatement,
        const Production* constStatement) {
        listTerm = list({
            capture(
                0,
                oneOf({
                    prod(commandStatement),
                    prod(forStatement),
                    prod(forEachStatement),
                    prod(whileStatement),
                    prod(doStatement),
                    prod(ifStatement),
                    prod(selectCaseStatement),
                    prod(tryStatement),
                    prod(dimStatement),
                    prod(dimCollectionStatement),
                    prod(constStatement),
                })),
        });
    }

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return std::move(captures->at(0));
    }
};

class TypeDeclarationProduction : public Production {
   public:
    explicit TypeDeclarationProduction(const Production* parameter)
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
                  zeroOrMore({
                      term(TokenKind::kEndOfLine),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<TypeDeclarationNode>(
            captureTokenText(std::move(captures->at(0))), captureNodeArray<ParameterNode>(std::move(captures->at(1))),
            firstToken);
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
                  zeroOrMore({
                      term(TokenKind::kEndOfLine),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<ProcedureNode>(
            captureTokenText(std::move(captures->at(0))), captureNodeArray<ParameterNode>(std::move(captures->at(1))),
            captureSingleNode<TypeNode>(std::move(captures->at(2))),
            captureSingleNode<BodyNode>(std::move(captures->at(3))), firstToken);
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
                  zeroOrMore({
                      term(TokenKind::kEndOfLine),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<ProcedureNode>(
            captureTokenText(std::move(captures->at(0))), captureNodeArray<ParameterNode>(std::move(captures->at(1))),
            captureSingleNode<BodyNode>(std::move(captures->at(2))), firstToken);
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
                  zeroOrMore({
                      term(TokenKind::kEndOfLine),
                  }),
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

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& /*firstToken*/) const override {
        return std::move(captures->at(0));
    }
};

class ProgramProduction : public Production {
   public:
    explicit ProgramProduction(const Production* member)
        : Production(
              NAMEOF_TYPE(ProgramProduction),
              {
                  zeroOrMore({
                      capture(0, prod(member)),
                  }),
              }) {}

    std::unique_ptr<Box> parse(CaptureArray* captures, const Token& firstToken) const override {
        return nodeBox<ProgramNode>(captureNodeArray<Node>(std::move(captures->at(0))), firstToken);
    }
};

class ProductionCollection {
   public:
    const Production* memberProduction;
    const Production* programProduction;

    ProductionCollection() {
        auto* statement = dynamic_cast<StatementProduction*>(add<StatementProduction>());
        auto* expression = dynamic_cast<ExpressionProduction*>(add<ExpressionProduction>());
        auto* type = dynamic_cast<TypeProduction*>(add<TypeProduction>());

        auto* body = add<BodyProduction>(statement);
        auto* argumentList = add<ArgumentListProduction>(expression);
        auto* parameter = add<ParameterProduction>(type);
        auto* parameterList = add<ParameterListProduction>(parameter);
        auto* namedType = add<NamedTypeProduction>();
        auto* typeWithParentheses = add<TypeWithParenthesesProduction>(type);
        auto* optionalType = add<OptionalTypeProduction>(typeWithParentheses);
        auto* mapType = add<MapTypeProduction>(typeWithParentheses);
        auto* setType = add<SetTypeProduction>(typeWithParentheses);
        auto* listType = add<ListTypeProduction>(typeWithParentheses);
        auto* recordType = add<RecordTypeProduction>(parameterList);
        auto* primitiveType = add<PrimitiveTypeProduction>();
        type->init(primitiveType, recordType, listType, mapType, setType, optionalType, namedType);
        auto* literalValue = add<LiteralValueProduction>();
        auto* literalRecordField = add<LiteralRecordFieldProduction>(expression);
        auto* literalRecordFieldList = add<LiteralRecordFieldListProduction>(literalRecordField);
        auto* literalRecordTerm = add<LiteralRecordTermProduction>(literalRecordFieldList);
        auto* literalArrayTerm = add<LiteralArrayTermProduction>(argumentList);
        auto* literalNoTerm = add<LiteralNoTermProduction>(type);
        auto* parenthesesTerm = add<ParenthesesTermProduction>(expression);
        auto* expressionTerm = add<ExpressionTermProduction>(
            literalValue, parenthesesTerm, literalArrayTerm, literalRecordTerm, literalNoTerm);
        auto* dottedExpressionSuffix = add<DottedExpressionSuffixProduction>(expression);
        auto* dottedExpression = add<DottedExpressionProduction>(expressionTerm, dottedExpressionSuffix);
        auto* convertExpression = add<ConvertExpressionProduction>(dottedExpression, type);
        auto* unaryExpression = add<UnaryExpressionProduction>(convertExpression);
        auto* exponentExpressionSuffix = add<ExponentExpressionSuffixProduction>(unaryExpression);
        auto* exponentExpression = add<ExponentExpressionProduction>(unaryExpression, exponentExpressionSuffix);
        auto* multiplyExpressionSuffix = add<MultiplyExpressionSuffixProduction>(exponentExpression);
        auto* multiplyExpression = add<MultiplyExpressionProduction>(exponentExpression, multiplyExpressionSuffix);
        auto* addExpressionSuffix = add<AddExpressionSuffixProduction>(multiplyExpression);
        auto* addExpression = add<AddExpressionProduction>(multiplyExpression, addExpressionSuffix);
        auto* inequalityExpressionSuffix = add<InequalityExpressionSuffixProduction>(addExpression);
        auto* inequalityExpression = add<InequalityExpressionProduction>(addExpression, inequalityExpressionSuffix);
        auto* equalityExpressionSuffix = add<EqualityExpressionSuffixProduction>(inequalityExpression);
        auto* equalityExpression = add<EqualityExpressionProduction>(inequalityExpression, equalityExpressionSuffix);
        auto* andExpressionSuffix = add<AndExpressionSuffixProduction>(equalityExpression);
        auto* andExpression = add<AndExpressionProduction>(equalityExpression, andExpressionSuffix);
        auto* orExpressionSuffix = add<OrExpressionSuffixProduction>(andExpression);
        auto* orExpression = add<OrExpressionProduction>(andExpression, orExpressionSuffix);
        expression->init(orExpression);
        auto* catchBlock = add<CatchBlockProduction>(body);
        auto* tryStatement = add<TryStatementProduction>(body, catchBlock);
        auto* rethrowStatement = add<RethrowStatementProduction>();
        auto* throwStatement = add<ThrowStatementProduction>(expression);
        auto* exitStatement = add<ExitStatementProduction>();
        auto* continueStatement = add<ContinueStatementProduction>();
        auto* callStatement = add<CallStatementProduction>(argumentList);
        auto* returnStatement = add<ReturnStatementProduction>(expression);
        auto* yieldStatement = add<YieldStatementProduction>(expression);
        auto* constStatement = add<ConstStatementProduction>(literalValue);
        auto* assignStatement = add<AssignStatementProduction>(expression, dottedExpression);
        auto* dimStatement = add<DimStatementProduction>(type, expression);
        auto* dimCollectionStatement = add<DimCollectionStatementProduction>(body);
        auto* caseValue = add<CaseValueProduction>(expression);
        auto* caseValueList = add<CaseValueListProduction>(caseValue);
        auto* case_ = add<CaseProduction>(caseValueList, body);
        auto* selectCaseStatement = add<SelectCaseStatementProduction>(expression, case_);
        auto* doStatement = add<DoStatementProduction>(expression, body);
        auto* elseIf = add<ElseIfProduction>(expression, body);
        auto* ifStatement = add<IfStatementProduction>(expression, body, elseIf, statement);
        auto* whileStatement = add<WhileStatementProduction>(expression, body);
        auto* forEachStatement = add<ForEachStatementProduction>(expression, body);
        auto* forStep = add<ForStepProduction>(expression);
        auto* forStatement = add<ForStatementProduction>(expression, forStep, body);
        auto* printStatement = add<PrintStatementProduction>(expression);
        auto* inputStatement = add<InputStatementProduction>(expression);
        auto* commandStatement = add<CommandStatementProduction>(
            assignStatement, yieldStatement, returnStatement, callStatement, continueStatement, exitStatement,
            throwStatement, rethrowStatement, printStatement, inputStatement);
        statement->init(
            commandStatement, forStatement, forEachStatement, whileStatement, doStatement, ifStatement,
            selectCaseStatement, tryStatement, dimStatement, dimCollectionStatement, constStatement);
        auto* typeDeclaration = add<TypeDeclarationProduction>(parameter);
        auto* function = add<FunctionProduction>(parameterList, type, body);
        auto* subroutine = add<SubroutineProduction>(parameterList, body);
        auto* member = add<MemberProduction>(subroutine, function, dimStatement, constStatement, typeDeclaration);
        auto* program = add<ProgramProduction>(member);

        memberProduction = member;
        programProduction = program;
    }

   private:
    std::vector<std::unique_ptr<Production>> _productions;

    template <typename T, typename... Args>
    Production* add(Args&&... args) {
        auto unique = std::make_unique<T>(std::forward<Args>(args)...);
        auto* production = unique.get();
        _productions.push_back(std::move(unique));
        return production;
    }
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static ProductionCollection _productionCollection;

class InputState {
   public:
    const SourceMember* sourceMember;
    const std::vector<Token>& tokens;
    size_t tokenIndex{ 0 };

    InputState(const std::vector<Token>& tokens, const SourceMember* sourceMember)
        : sourceMember(sourceMember),
          tokens(tokens),

          _endOfFileToken(Token(-1, 0, TokenKind::kEndOfFile, "", sourceMember)) {}

    const Token& currentToken() {
        if (tokenIndex < tokens.size()) {
            return tokens.at(tokenIndex);
        }
        return _endOfFileToken;
    }

   private:
    const Token _endOfFileToken;
};

class ProductionState {
   public:
    CaptureArray captures;
    Token firstToken;

    explicit ProductionState(Token firstToken) : firstToken(std::move(firstToken)) {
        for (auto i = 0; i < kNumCaptures; i++) {
            captures.at(i) = nullptr;
        }
    }

    std::array<int, kNumCaptures> captureCounts() const {
        std::array<int, kNumCaptures> counts{};
        for (auto i = 0; i < kNumCaptures; i++) {
            if (captures.at(i)) {
                counts.at(i) = static_cast<int>(captures.at(i)->count());
            } else {
                counts.at(i) = 0;
            }
        }
        return counts;
    }
};

class Checkpoint {
   public:
    Checkpoint(const InputState* inputState, const ProductionState& productionState)
        : _tokenIndex(inputState->tokenIndex), _captureCounts(productionState.captureCounts()) {}

    void revert(InputState* inputState, ProductionState* productionState) {
        inputState->tokenIndex = _tokenIndex;
        for (auto i = 0; i < kNumCaptures; i++) {
            trimCaptures(productionState->captures.at(i).get(), _captureCounts.at(i));
        }
    }

   private:
    const size_t _tokenIndex;
    const std::array<int, kNumCaptures> _captureCounts;

    static void trimCaptures(Box* box, size_t desiredCount) {
        if (box != nullptr) {
            while (box->count() > desiredCount) {
                box->removeAt(static_cast<ptrdiff_t>(box->count()) - 1);
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

    TermResult(
        bool isMatch,
        bool isError,
        const Term* mismatchedTerm,
        std::optional<Token> token,
        std::unique_ptr<Box> box)
        : isMatch(isMatch),
          isError(isError),
          mismatchedTerm(mismatchedTerm),
          token(std::move(token)),
          box(std::move(box)) {}
};

class ParseStackFrame {
   public:
    // either term or production
    const Term* term = nullptr;
    const Production* production = nullptr;

    boost::local_shared_ptr<ProductionState> productionState = nullptr;
    std::unique_ptr<Checkpoint> checkpoint = nullptr;
    std::unique_ptr<TermResult> subTermResult = nullptr;
    std::unique_ptr<Checkpoint> elementCheckpoint = nullptr;  // when term->type == kZeroOrMore
    bool cutHasBeenHit = false;                               // when term->type == kAnd or kOptional

    size_t step = 0;

    ParseStackFrame()
        : productionState(boost::make_local_shared<ProductionState>(Token(0, 0, TokenKind::kEndOfFile, "", nullptr))) {}
    ParseStackFrame(
        const Production& production,
        InputState* inputState,
        const boost::local_shared_ptr<ProductionState>& productionState)
        : production(&production),
          checkpoint(std::make_unique<Checkpoint>(inputState, *productionState)),
          productionState(productionState) {}
    ParseStackFrame(
        const Term& term,
        InputState* inputState,
        const boost::local_shared_ptr<ProductionState>& productionState)
        : term(&term),
          checkpoint(std::make_unique<Checkpoint>(inputState, *productionState)),
          productionState(productionState) {}
};

static Token peekToken(const InputState* inputState) {
    if (inputState->tokenIndex < inputState->tokens.size()) {
        return inputState->tokens.at(inputState->tokenIndex);
    }
    return { -1, 0, TokenKind::kEndOfFile, "", inputState->sourceMember };
}

static void acceptToken(InputState* inputState) {
    inputState->tokenIndex++;
}

static std::unique_ptr<Box> appendCapture(std::unique_ptr<Box> existingBox, std::unique_ptr<Box> newBox) {
    if (existingBox) {
        switch (existingBox->type) {
            case BoxType::kNode: {
                if (!existingBox) {
                    existingBox = std::make_unique<NodeBox>();
                }
                auto* existingNodeBox = dynamic_cast_borrow<NodeBox>(existingBox);
                auto* newNodeBox = dynamic_cast_borrow<NodeBox>(newBox);
                existingNodeBox->append(newNodeBox);
                break;
            }

            case BoxType::kToken: {
                if (!existingBox) {
                    existingBox = std::make_unique<TokenBox>();
                }
                auto* existingTokenBox = dynamic_cast_borrow<TokenBox>(existingBox);
                auto* newTokenBox = dynamic_cast_borrow<TokenBox>(newBox);
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
    return existingBox;
}

static void pumpParseAnd(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>* stack,
    InputState* inputState) {
    // step 0: start subTerms[0]
    // step 1: receive subTerms[0], start subTerms[1]
    // ...
    // step N: receive subTerms[n-1], return

    if (frame->step > 0) {
        // receive subTerms[step-1]
        if (!frame->subTermResult->isMatch) {
            auto result = std::move(frame->subTermResult);
            frame->checkpoint->revert(inputState, frame->productionState.get());
            if (frame->cutHasBeenHit) {
                result->isError = true;
            }
            stack->pop();
            stack->top()->subTermResult = std::move(result);
            return;
        }
    }

    if (frame->step < frame->term->subTerms.size()) {
        // start subTerms[step]
        const auto& subTerm = frame->term->subTerms.at(frame->step);
        stack->push(std::make_unique<ParseStackFrame>(subTerm, inputState, frame->productionState));
        frame->subTermResult = nullptr;
        frame->step++;
    } else {
        // return
        stack->pop();
        stack->top()->subTermResult = TermResult::newSuccess();
    }
}

static void pumpParseCapture(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>* stack,
    InputState* inputState) {
    // step 0: start subTerms[0]
    // step 1: receive subTerms[0], return

    if (frame->step == 0) {
        // start subTerms[0]
        const auto& subTerm = frame->term->subTerms.at(0);
        stack->push(std::make_unique<ParseStackFrame>(subTerm, inputState, frame->productionState));
        frame->subTermResult = nullptr;
        frame->step++;
    } else {
        // receive subTerms[0]
        if (frame->subTermResult->isMatch) {
            frame->productionState->captures.at(frame->term->captureId) = appendCapture(
                std::move(frame->productionState->captures.at(frame->term->captureId)),
                std::move(frame->subTermResult->box));
            stack->pop();
            stack->top()->subTermResult = TermResult::newSuccess();
        } else {
            auto result = std::move(frame->subTermResult);
            stack->pop();
            stack->top()->subTermResult = std::move(result);
        }
    }
}

static void pumpParseCut(ParseStackFrame* frame, std::stack<std::unique_ptr<ParseStackFrame>>* stack) {
    // single step
    frame->cutHasBeenHit = true;
    stack->pop();
    stack->top()->subTermResult = TermResult::newSuccess();
}

static void pumpParseNonTerminal(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>* stack,
    InputState* inputState) {
    // step 0: start sub-production
    // step 1: receive sub-production, return

    if (frame->step == 0) {
        // start sub-production
        stack->push(std::make_unique<ParseStackFrame>(
            *frame->term->production, inputState, std::make_unique<ProductionState>(peekToken(inputState))));
        frame->subTermResult = nullptr;
        frame->step++;
    } else {
        // receive sub-production
        auto result = std::move(frame->subTermResult);

        // return
        stack->pop();
        stack->top()->subTermResult = std::move(result);
    }
}

static void pumpParseOptional(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>* stack,
    InputState* inputState) {
    // step 0: start subTerms[0]
    // step 1: receive subTerms[0], start subTerms[1]
    // ...
    // step N: receive subTerms[n-1], return

    if (frame->step > 0) {
        // receive subTerms[step-1]
        if (!frame->subTermResult->isMatch) {
            auto result = std::move(frame->subTermResult);
            frame->checkpoint->revert(inputState, frame->productionState.get());
            auto cutHasBeenHit = frame->cutHasBeenHit;
            stack->pop();
            if (cutHasBeenHit) {
                result->isError = true;
                stack->top()->subTermResult = std::move(result);
            } else {
                // if it didn't match (but isn't an error), that's fine because this was optional
                stack->top()->subTermResult = result->isError ? std::move(result) : TermResult::newSuccess();
            }
            return;
        }
    }

    if (frame->step < frame->term->subTerms.size()) {
        // start subTerms[step]
        const auto& subTerm = frame->term->subTerms.at(frame->step);
        stack->push(std::make_unique<ParseStackFrame>(subTerm, inputState, frame->productionState));
        frame->subTermResult = nullptr;
        frame->step++;
    } else {
        // return
        stack->pop();
        stack->top()->subTermResult = TermResult::newSuccess();
    }
}

static void pumpParseOr(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>* stack,
    InputState* inputState) {
    // step 0: start subTerms[0]
    // step 1: receive subTerms[0], start subTerms[1]
    // ...
    // step N: receive subTerms[n-1], return

    if (frame->step > 0) {
        // receive subTerms[step-1]
        if (frame->subTermResult->isError || frame->subTermResult->isMatch) {
            auto result = std::move(frame->subTermResult);
            stack->pop();
            stack->top()->subTermResult = std::move(result);
            return;
        }
        auto result = std::move(frame->subTermResult);
        frame->checkpoint->revert(inputState, frame->productionState.get());
        // it's fine, try the next one
    }

    if (frame->step < frame->term->subTerms.size()) {
        // start subTerms[step]
        const auto& subTerm = frame->term->subTerms.at(frame->step);
        stack->push(std::make_unique<ParseStackFrame>(subTerm, inputState, frame->productionState));
        frame->subTermResult = nullptr;
        frame->step++;
    } else {
        // if we made it this far, then none matched. return
        auto result = TermResult::newMismatch(*frame->term, inputState->currentToken());
        stack->pop();
        stack->top()->subTermResult = std::move(result);
    }
}

static void pumpParseTerminal(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>* stack,
    InputState* inputState) {
    // single step
    auto terminalToken = peekToken(inputState);
    if (frame->term->tokenKind == terminalToken.type) {
        acceptToken(inputState);
        stack->pop();
        stack->top()->subTermResult = TermResult::newSuccess(tokenBox(terminalToken));
    } else {
        frame->checkpoint->revert(inputState, frame->productionState.get());
        auto result = TermResult::newMismatch(*frame->term, inputState->currentToken());
        stack->pop();
        stack->top()->subTermResult = std::move(result);
    }
}

static void pumpParseZeroOrMore(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>* stack,
    InputState* inputState) {
    // step 0: start subTerms[0]
    // step N: receive subTerms[0], start subTerms[0] again or return.

    if (frame->step > 0) {
        // receive subTerms[0]
        if (!frame->subTermResult->isMatch) {
            frame->elementCheckpoint->revert(inputState, frame->productionState.get());
            if (frame->subTermResult->isError) {
                // it matched up to the cut, so this is an error
                auto result = std::move(frame->subTermResult);
                stack->pop();
                stack->top()->subTermResult = std::move(result);
            } else {
                // it's ok, end of the list
                stack->pop();
                stack->top()->subTermResult = TermResult::newSuccess();
            }
            return;
        }
    }

    // start subTerms[0]
    frame->elementCheckpoint = std::make_unique<Checkpoint>(inputState, *frame->productionState);
    stack->push(std::make_unique<ParseStackFrame>(frame->term->subTerms.at(0), inputState, frame->productionState));
    frame->subTermResult = nullptr;
    frame->step++;
}

static void pumpParseProduction(
    ParseStackFrame* frame,
    std::stack<std::unique_ptr<ParseStackFrame>>* stack,
    InputState* inputState) {
    // step 0: start listTerm
    // step 1: receive listTerm

    if (frame->step == 0) {
        // start listTerm
        stack->push(std::make_unique<ParseStackFrame>(frame->production->listTerm, inputState, frame->productionState));
        frame->subTermResult = nullptr;
        frame->step++;
    } else {
        // receive listTerm
        if (frame->subTermResult->isMatch) {
            auto termResult = TermResult::newSuccess(
                frame->production->parse(&frame->productionState->captures, frame->productionState->firstToken));
            if (frame->cutHasBeenHit) {
                termResult->isError = true;
            }
            stack->pop();
            stack->top()->subTermResult = std::move(termResult);
        } else {
            auto result = std::move(frame->subTermResult);
            stack->pop();
            stack->top()->subTermResult = std::move(result);
        }
    }
}

static void pumpParse(std::stack<std::unique_ptr<ParseStackFrame>>* stack, InputState* inputState) {
    auto* frame = stack->top().get();

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

    for (size_t i = 0; i < stack->size(); i++) {
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

    if (frame->production != nullptr) {
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
                pumpParseCut(frame, stack);
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

static ParserResult parseRootProduction(
    const SourceMember* sourceMember,
    const Production& production,
    const std::vector<Token>& tokens) {
    auto inputState = InputState(tokens, sourceMember);
    auto stack = std::stack<std::unique_ptr<ParseStackFrame>>();

    stack.push(std::make_unique<ParseStackFrame>());

    stack.push(std::make_unique<ParseStackFrame>(
        production, &inputState, std::make_unique<ProductionState>(peekToken(&inputState))));

    while (stack.size() > 1) {
        pumpParse(&stack, &inputState);
    }

#ifdef DUMP_PARSE
    std::cerr << std::endl;
#endif

    auto& result = stack.top()->subTermResult;
    if (result->isMatch) {
        if (inputState.tokenIndex < inputState.tokens.size()) {
            // the code is good but there is unparsed junk left over
            return { "This token was unexpected.", inputState.tokens.at(inputState.tokenIndex) };
        }
        auto root = captureSingleNode<Node>(std::move(result->box));
#ifdef DUMP_AST
        root->dump(std::cerr, 0);
#endif
        return ParserResult(std::move(root));
    }
    return { "This token was unexpected.", inputState.tokens.at(inputState.tokenIndex) };
}

ParserResult parse(
    const SourceMember* sourceMember,
    ParserRootProduction rootProduction,
    const std::vector<Token>& tokens) {
    switch (rootProduction) {
        case ParserRootProduction::kMember:
            return parseRootProduction(sourceMember, *_productionCollection.memberProduction, tokens);
        case ParserRootProduction::kProgram:
            return parseRootProduction(sourceMember, *_productionCollection.programProduction, tokens);
        default:
            return { "Invalid root production.", Token(0, 0, TokenKind::kError, "", sourceMember) };
    }
}

}  // namespace compiler
