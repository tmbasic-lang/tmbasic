#include "Parser.h"

using namespace basic;
using namespace compiler;

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
static std::vector<std::unique_ptr<T>> captureNodeArray(std::unique_ptr<Box>& box) {
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

static Token captureToken(std::unique_ptr<Box>& box) {
    assert(box->type == BoxType::kToken);
    auto tokenBox = cast<TokenBox>(std::move(box));
    return tokenBox->values[0];
}

static std::string captureTokenText(std::unique_ptr<Box>& box) {
    return captureToken(box).text;
}

/*static TokenType captureTokenType(std::unique_ptr<Box>& box) {
    return captureToken(box).type;
}*/

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
        auto statements = captureNodeArray<StatementNode>(captures[0]);
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
        return nodeBox(new ParameterNode(captureTokenText(captures[0]), std::move(type), firstToken));
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
        type->recordName = std::optional(captureTokenText(captures[0]));
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
        for (auto& parameter : captureNodeArray<ParameterNode>(captures[0])) {
            auto field = new FieldNode(parameter->name, std::move(parameter->type), firstToken);
            type->fields.push_back(std::unique_ptr<FieldNode>(field));
        }
        return nodeBox(type);
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

class TypeProduction : public Production {
   public:
    TypeProduction() : Production({}) {}

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
        auto type = add(new TypeProduction());

        add(new BodyProduction(statement));
        add(new ArgumentListProduction(expression));
        auto parameter = add(new ParameterProduction(type));
        auto parameterList = add(new ParameterListProduction(parameter));
        add(new NamedTypeProduction());
        auto typeWithParentheses = add(new TypeWithParenthesesProduction(type));
        add(new OptionalTypeProduction(typeWithParentheses));
        add(new MapTypeProduction(typeWithParentheses));
        add(new ListTypeProduction(typeWithParentheses));
        add(new RecordTypeProduction(parameterList));

        // TODO: init statement, expression, type
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
