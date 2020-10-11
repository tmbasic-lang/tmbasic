#include "Parser.h"
#include "core/util/decimal.h"

using namespace basic;
using namespace compiler;
using namespace util;

enum class TermType { kAnd, kCapture, kCut, kNonTerminal, kOptional, kOr, kTerminal, kZeroOrMore };

class Production;

class Term {
   public:
    TermType type;
    std::vector<Term> subTerms;
    const Production* production;  // when type = kNonTerminal
    basic::TokenType tokenType;    // when type = kTerminal
    int captureId;                 // when type = kCapture
    Term(TermType type) : type(type){};
};

enum class BoxType { kNode, kToken, kType };

class Box {
   public:
    BoxType type;
    Box(BoxType type) : type(type) {}
    virtual ~Box() = 0;
    virtual size_t count() = 0;
};

Box::~Box() {}

class NodeBox : public Box {
   public:
    std::vector<std::unique_ptr<Node>> values;
    NodeBox() : Box(BoxType::kNode) {}
    size_t count() override { return values.size(); }
};

class TokenBox : public Box {
   public:
    std::vector<Token> values;
    TokenBox() : Box(BoxType::kToken) {}
    size_t count() override { return values.size(); }
};

class Production {
   public:
    std::vector<Term> terms;
    Production(std::initializer_list<Term> terms) : terms(terms) {}
    virtual std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) = 0;
};

// dynamic_cast and move into a new unique_ptr
template <typename TDst, typename TSrc>
static std::unique_ptr<TDst> cast(std::unique_ptr<TSrc> src) {
    if (!src) {
        return {};
    } else {
        TDst* dst = &dynamic_cast<TDst&>(*src.get());
        src.release();
        return std::unique_ptr<TDst>(dst);
    }
}

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
    assert(box->type == BoxType::kNode);
    std::vector<std::unique_ptr<T>> items;
    auto nodeBox = cast<NodeBox>(std::move(box));
    for (auto& x : nodeBox->values) {
        items.push_back(cast<T>(std::move(x)));
    }
    return items;
}

template <typename T>
static std::unique_ptr<T> captureSingleNode(std::unique_ptr<Box> box) {
    assert(box->type == BoxType::kNode);
    auto nodeBox = cast<NodeBox>(std::move(box));
    return cast<T>(std::move(nodeBox->values[0]));
}

static bool hasCapture(std::unique_ptr<Box>& box) {
    return box->count() > 0;
}

template <typename T>
static std::unique_ptr<T> captureSingleNodeOrNull(std::unique_ptr<Box> box) {
    if (hasCapture(box)) {
        return captureSingleNode<T>(box);
    } else {
        return {};
    }
}

static Token captureToken(std::unique_ptr<Box> box) {
    assert(box->type == BoxType::kToken);
    auto tokenBox = cast<TokenBox>(std::move(box));
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

/*static std::unique_ptr<NodeBox> nodeBox(std::vector<std::unique_ptr<Node>>& node) {
    auto box = std::make_unique<NodeBox>();
    box->values = std::move(node);
    return box;
}*/

/*static std::unique_ptr<TokenBox> tokenBox(Token token) {
    auto box = std::make_unique<TokenBox>();
    box->values.push_back(token);
    return box;
}*/

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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
        return std::move(captures[0]);
    }
};

class NamedTypeProduction : public Production {
   public:
    NamedTypeProduction()
        : Production({
              capture(0, term(TokenType::kIdentifier)),
          }) {}

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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
              optional({ term(TokenType::kLeftParenthesis), capture(1, prod(argumentList)),
                         term(TokenType::kRightParenthesis) }),
          }) {}

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
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

class StatementProduction : public Production {
   public:
    StatementProduction() : Production({}) {}

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
        return {};
    }
};

class ExpressionProduction : public Production {
   public:
    ExpressionProduction() : Production({}) {}

    std::unique_ptr<Box> parse(std::vector<std::unique_ptr<Box>>& captures, const Token& firstToken) override {
        return {};
    }
};

namespace compiler {
class ProductionCollection {
   public:
    ProductionCollection() {
        auto statement = add(new StatementProduction());
        auto expression = add(new ExpressionProduction());
        auto type = dynamic_cast<TypeProduction*>(add(new TypeProduction()));

        add(new BodyProduction(statement));
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
        add(new DottedExpressionProduction(expressionTerm, dottedExpressionSuffix));

        // TODO: init statement, expression
    }

   private:
    std::vector<std::unique_ptr<Production>> _productions;

    Production* add(Production* production) {
        _productions.push_back(std::unique_ptr<Production>(production));
        return production;
    }
};
}  // namespace compiler

Parser::Parser() : _productionCollection(std::make_unique<ProductionCollection>()) {}

std::unique_ptr<basic::ProcedureNode> Parser::parseProcedure(const std::vector<basic::Token>& tokens) {
    return {};
}
