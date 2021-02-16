#include "compiler/ast.h"
#include "util/decimal.h"

using util::decimalToString;

namespace compiler {

static std::string indent(int n) {
    return std::string(n, ' ');
}

static std::string stripNamespace(std::string_view sv) {
    auto s = std::string(sv);
    return std::regex_replace(s, std::regex("^"), "");
}

// NOLINTNEXTLINE
#define DUMP_VAR_NAME(f) \
    { s << indent(n + 1) << NAMEOF(f) << '\n'; }  // NOLINT

// NOLINTNEXTLINE
#define DUMP_NODES(f)         \
    for (const auto& x : f) { \
        x->dump(s, n + 2);    \
    }  // NOLINT

// NOLINTNEXTLINE
#define DUMP_TYPE(t) \
    { s << indent(n) << stripNamespace(NAMEOF_TYPE(t)) << '\n'; }  // NOLINT

// NOLINTNEXTLINE
#define DUMP_VAR(f) \
    { s << indent(n + 1) << NAMEOF(f) << '=' << f << '\n'; }  // NOLINT

template <typename T>
static void dumpVarOptional(std::ostringstream& s, const std::optional<T>& f, int n) {
    if (f.has_value()) {
        s << indent(n + 1) << NAMEOF(f) << '=' << *f << '\n';  // NOLINT
    }
}

// NOLINTNEXTLINE
#define DUMP_VAR_OPTIONAL(f) dumpVarOptional(s, f, n)

// NOLINTNEXTLINE
#define DUMP_VAR_DECIMAL(f) \
    { s << indent(n + 1) << NAMEOF(f) << '=' << decimalToString(f) << '\n'; }  // NOLINT

static void dumpVarOptionalDecimal(std::ostringstream& s, const std::optional<decimal::Decimal>& f, int n) {
    if (f.has_value()) {
        s << indent(n + 1) << NAMEOF(f) << '=' << decimalToString(*f) << '\n';  // NOLINT
    }
}

// NOLINTNEXTLINE
#define DUMP_VAR_OPTIONAL_DECIMAL(f) dumpVarOptionalDecimal(s, f, n)

// NOLINTNEXTLINE
#define DUMP_VAR_ENUM(f) \
    { s << indent(n + 1) << NAMEOF(f) << '=' << NAMEOF_ENUM(f) << '\n'; }  // NOLINT

// NOLINTNEXTLINE
#define DUMP_VAR_NODE(f)   \
    if (f) {               \
        DUMP_VAR_NAME(f);  \
        f->dump(s, n + 2); \
    }  // NOLINT

// NOLINTNEXTLINE
#define DUMP_VAR_NODES(f) \
    if (!f.empty()) {     \
        DUMP_VAR_NAME(f); \
        DUMP_NODES(f);    \
    }

Node::Node(Token token) : token(std::move(token)) {}

Node::~Node() = default;

void Node::dump(std::ostringstream& s, int n) const {
    assert(false);
    DUMP_TYPE(Node);
}

MemberType Node::getMemberType() const {
    return MemberType::kNonMember;
}

std::optional<std::string> Node::getSymbolDeclaration() const {
    return std::optional<std::string>();
}

Node* Node::getChildSymbolDeclaration() const {
    return nullptr;
}

bool Node::isSymbolVisibleToSiblingStatements() const {
    return false;
}

bool Node::visitBodies(const VisitBodyFunc& /*func*/) const {
    return true;
}

bool Node::visitExpressions(const VisitExpressionFunc& /*func*/) const {
    return true;
}

bool Node::isSymbolReference() const {
    return false;
}

TypeNode* Node::getChildTypeNode() const {
    return nullptr;
}

ExpressionNode::ExpressionNode(Token token) : Node(std::move(token)) {}

ConstValueExpressionNode::ConstValueExpressionNode(Token token) : ExpressionNode(std::move(token)) {}

StatementNode::StatementNode(StatementType type, Token token) : type(type), Node(std::move(token)) {}

FieldNode::FieldNode(std::string name, std::unique_ptr<TypeNode> type, Token token)
    : Node(std::move(token)), name(std::move(name)), type(std::move(type)) {}

void FieldNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(FieldNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(type);
}

TypeNode::TypeNode(Kind kind, Token token) : Node(std::move(token)), kind(kind) {}

TypeNode::TypeNode(Kind kind, Token token, std::string recordName)
    : Node(std::move(token)), kind(kind), recordName(recordName) {}

TypeNode::TypeNode(Kind kind, Token token, std::unique_ptr<TypeNode> optionalValueTypeOrListItemType)
    : Node(std::move(token)),
      kind(kind),
      listItemType(kind == Kind::kList ? std::move(optionalValueTypeOrListItemType) : std::unique_ptr<TypeNode>()),
      optionalValueType(
          kind == Kind::kOptional ? std::move(optionalValueTypeOrListItemType) : std::unique_ptr<TypeNode>()) {}

TypeNode::TypeNode(Kind kind, Token token, std::unique_ptr<TypeNode> mapKeyType, std::unique_ptr<TypeNode> mapValueType)
    : Node(std::move(token)), kind(kind), mapKeyType(std::move(mapKeyType)), mapValueType(std::move(mapValueType)) {}

TypeNode::TypeNode(Kind kind, Token token, std::vector<std::unique_ptr<FieldNode>> fields)
    : Node(std::move(token)), kind(kind), fields(std::move(fields)) {}

void TypeNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(TypeNode);
    DUMP_VAR_ENUM(kind);
    DUMP_VAR_OPTIONAL(recordName);
    DUMP_VAR_OPTIONAL(genericPlaceholderName);
    DUMP_VAR_NODES(fields);
    DUMP_VAR_NODE(listItemType);
    DUMP_VAR_NODE(mapKeyType);
    DUMP_VAR_NODE(mapValueType);
    DUMP_VAR_NODE(optionalValueType);
}

BinaryExpressionSuffixNode::BinaryExpressionSuffixNode(
    BinaryOperator binaryOperator,
    std::unique_ptr<ExpressionNode> rightOperand,
    Token token)
    : Node(std::move(token)), binaryOperator(binaryOperator), rightOperand(std::move(rightOperand)) {}

void BinaryExpressionSuffixNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(BinaryExpressionSuffixNode);
    DUMP_VAR_ENUM(binaryOperator);
    DUMP_VAR_NODE(rightOperand);
}

bool BinaryExpressionSuffixNode::visitExpressions(const VisitExpressionFunc& func) const {
    return func(*rightOperand);
}

BinaryExpressionNode::BinaryExpressionNode(
    std::unique_ptr<ExpressionNode> leftOperand,
    std::vector<std::unique_ptr<BinaryExpressionSuffixNode>> binarySuffixes,
    Token token)
    : ExpressionNode(std::move(token)),
      leftOperand(std::move(leftOperand)),
      binarySuffixes(std::move(binarySuffixes)) {}

void BinaryExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(BinaryExpressionNode);
    DUMP_VAR_NODE(leftOperand);
    DUMP_VAR_NODES(binarySuffixes);
}

bool BinaryExpressionNode::visitExpressions(const VisitExpressionFunc& func) const {
    if (!func(*leftOperand)) {
        return false;
    }
    for (const auto& x : binarySuffixes) {
        if (!x->visitExpressions(func)) {
            return false;
        }
    }
    return true;
}

CallExpressionNode::CallExpressionNode(
    std::string name,
    std::vector<std::unique_ptr<ExpressionNode>> arguments,
    Token token)
    : ExpressionNode(std::move(token)), name(std::move(name)), arguments(std::move(arguments)) {}

void CallExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(CallExpressionNode);
    DUMP_VAR(name);
    DUMP_VAR_NODES(arguments);
}

bool CallExpressionNode::visitExpressions(const VisitExpressionFunc& func) const {
    for (const auto& x : arguments) {
        if (!x->visitExpressions(func)) {
            return false;
        }
    }
    return true;
}

ConvertExpressionNode::ConvertExpressionNode(
    std::unique_ptr<ExpressionNode> value,
    std::unique_ptr<TypeNode> type,
    Token token)
    : ExpressionNode(std::move(token)), value(std::move(value)), type(std::move(type)) {}

void ConvertExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ConvertExpressionNode);
    DUMP_VAR_NODE(value);
    DUMP_VAR_NODE(type);
}

bool ConvertExpressionNode::visitExpressions(const VisitExpressionFunc& func) const {
    return func(*value);
}

TypeNode* ConvertExpressionNode::getChildTypeNode() const {
    return type.get();
}

DottedExpressionSuffixNode::DottedExpressionSuffixNode(
    std::string name,
    bool isCall,
    std::vector<std::unique_ptr<ExpressionNode>> callArguments,
    Token token)
    : Node(std::move(token)), name(std::move(name)), isCall(isCall), callArguments(std::move(callArguments)) {}

void DottedExpressionSuffixNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DottedExpressionSuffixNode);
    DUMP_VAR(name);
    DUMP_VAR(isCall);
    DUMP_VAR_NODES(callArguments);
}

bool DottedExpressionSuffixNode::visitExpressions(const VisitExpressionFunc& func) const {
    for (const auto& x : callArguments) {
        if (!func(*x)) {
            return false;
        }
    }
    return true;
}

DottedExpressionNode::DottedExpressionNode(
    std::unique_ptr<ExpressionNode> base,
    std::vector<std::unique_ptr<DottedExpressionSuffixNode>> dottedSuffixes,
    Token token)
    : ExpressionNode(std::move(token)), base(std::move(base)), dottedSuffixes(std::move(dottedSuffixes)) {}

void DottedExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DottedExpressionNode);
    DUMP_VAR_NODE(base);
    DUMP_VAR_NODES(dottedSuffixes);
}

bool DottedExpressionNode::visitExpressions(const VisitExpressionFunc& func) const {
    if (!func(*base)) {
        return false;
    }
    for (const auto& x : dottedSuffixes) {
        if (!x->visitExpressions(func)) {
            return false;
        }
    }
    return true;
}

LiteralArrayExpressionNode::LiteralArrayExpressionNode(
    std::vector<std::unique_ptr<ExpressionNode>> elements,
    Token token)
    : ConstValueExpressionNode(std::move(token)), elements(std::move(elements)) {}

void LiteralArrayExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(LiteralArrayExpressionNode);
    DUMP_VAR_NODES(elements);
}

bool LiteralArrayExpressionNode::visitExpressions(const VisitExpressionFunc& func) const {
    for (const auto& x : elements) {
        if (!func(*x)) {
            return false;
        }
    }
    return true;
}

LiteralBooleanExpressionNode::LiteralBooleanExpressionNode(bool value, Token token)
    : ConstValueExpressionNode(std::move(token)), value(value) {}

void LiteralBooleanExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(LiteralBooleanExpressionNode);
    DUMP_VAR(value);
}

LiteralNumberExpressionNode::LiteralNumberExpressionNode(decimal::Decimal value, Token token)
    : ConstValueExpressionNode(std::move(token)), value(std::move(value)) {}

void LiteralNumberExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(LiteralNumberExpressionNode);
    DUMP_VAR_DECIMAL(value);
}

LiteralRecordFieldNode::LiteralRecordFieldNode(std::string key, std::unique_ptr<ExpressionNode> value, Token token)
    : Node(std::move(token)), key(std::move(key)), value(std::move(value)) {}

void LiteralRecordFieldNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(LiteralRecordFieldNode);
    DUMP_VAR(key);
    DUMP_VAR_NODE(value);
}

bool LiteralRecordFieldNode::visitExpressions(const VisitExpressionFunc& func) const {
    return func(*value);
}

LiteralRecordExpressionNode::LiteralRecordExpressionNode(
    std::vector<std::unique_ptr<LiteralRecordFieldNode>> fields,
    Token token)
    : ConstValueExpressionNode(std::move(token)), fields(std::move(fields)) {}

void LiteralRecordExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(LiteralRecordExpressionNode);
    DUMP_VAR_NODES(fields);
}

bool LiteralRecordExpressionNode::visitExpressions(const VisitExpressionFunc& func) const {
    for (const auto& x : fields) {
        if (!x->visitExpressions(func)) {
            return false;
        }
    }
    return true;
}

LiteralStringExpressionNode::LiteralStringExpressionNode(std::string value, Token token)
    : ConstValueExpressionNode(std::move(token)), value(std::move(value)) {}

void LiteralStringExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(LiteralStringExpressionNode);
    DUMP_VAR(value);
}

NotExpressionNode::NotExpressionNode(std::unique_ptr<ExpressionNode> operand, Token token)
    : ExpressionNode(std::move(token)), operand(std::move(operand)) {}

void NotExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(NotExpressionNode);
    DUMP_VAR_NODE(operand);
}

bool NotExpressionNode::visitExpressions(const VisitExpressionFunc& func) const {
    return func(*operand);
}

SymbolReferenceExpressionNode::SymbolReferenceExpressionNode(std::string name, Token token)
    : ExpressionNode(std::move(token)), name(std::move(name)) {}

void SymbolReferenceExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(SymbolReferenceExpressionNode);
    DUMP_VAR(name);
}

bool SymbolReferenceExpressionNode::isSymbolReference() const {
    return true;
}

AssignLocationSuffixNode::AssignLocationSuffixNode(std::string name, Token token)
    : Node(std::move(token)), arrayIndex(std::unique_ptr<ExpressionNode>()), name(std::optional<std::string>(name)) {}

AssignLocationSuffixNode::AssignLocationSuffixNode(std::unique_ptr<ExpressionNode> arrayIndex, Token token)
    : Node(std::move(token)), arrayIndex(std::move(arrayIndex)), name({}) {}

void AssignLocationSuffixNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(AssignLocationSuffixNode);
    DUMP_VAR_OPTIONAL(name);
    DUMP_VAR_NODE(arrayIndex);
}

bool AssignLocationSuffixNode::visitExpressions(const VisitExpressionFunc& func) const {
    if (arrayIndex) {
        if (!func(*arrayIndex)) {
            return false;
        }
    }
    return true;
}

AssignStatementNode::AssignStatementNode(
    std::string name,
    std::vector<std::unique_ptr<AssignLocationSuffixNode>> suffixes,
    std::unique_ptr<ExpressionNode> value,
    Token token)
    : StatementNode(StatementType::kAssign, std::move(token)),
      name(std::move(name)),
      suffixes(std::move(suffixes)),
      value(std::move(value)) {}

void AssignStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(AssignStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODES(suffixes);
    DUMP_VAR_NODE(value);
}

bool AssignStatementNode::visitExpressions(const VisitExpressionFunc& func) const {
    for (const auto& x : suffixes) {
        if (!x->visitExpressions(func)) {
            return false;
        }
    }
    return func(*value);
}

BodyNode::BodyNode(std::vector<std::unique_ptr<StatementNode>> statements, Token token)
    : Node(std::move(token)), statements(std::move(statements)) {}

void BodyNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(BodyNode);
    DUMP_VAR_NODES(statements);
}

CallStatementNode::CallStatementNode(
    std::string name,
    std::vector<std::unique_ptr<ExpressionNode>> arguments,
    Token token)
    : StatementNode(StatementType::kCall, std::move(token)), name(std::move(name)), arguments(std::move(arguments)) {}

void CallStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(CallStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODES(arguments);
}

bool CallStatementNode::visitExpressions(const VisitExpressionFunc& func) const {
    for (const auto& x : arguments) {
        if (!func(*x)) {
            return false;
        }
    }
    return true;
}

ConstStatementNode::ConstStatementNode(std::string name, std::unique_ptr<ConstValueExpressionNode> value, Token token)
    : StatementNode(StatementType::kConst, std::move(token)), name(std::move(name)), value(std::move(value)) {}

MemberType ConstStatementNode::getMemberType() const {
    return MemberType::kConstStatement;
}

void ConstStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ConstStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(value);
}

std::optional<std::string> ConstStatementNode::getSymbolDeclaration() const {
    return name;
}

ContinueStatementNode::ContinueStatementNode(ContinueScope scope, Token token)
    : StatementNode(StatementType::kContinue, std::move(token)), scope(scope) {}

void ContinueStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ContinueStatementNode);
    DUMP_VAR_ENUM(scope);
}

DimListStatementNode::DimListStatementNode(std::string name, std::unique_ptr<BodyNode> body, Token token)
    : StatementNode(StatementType::kDimList, std::move(token)), name(std::move(name)), body(std::move(body)) {}

void DimListStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DimListStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(body);
}

bool DimListStatementNode::visitBodies(const VisitBodyFunc& func) const {
    return func(*body);
}

DimMapStatementNode::DimMapStatementNode(std::string name, std::unique_ptr<BodyNode> body, Token token)
    : StatementNode(StatementType::kDimMap, std::move(token)), name(std::move(name)), body(std::move(body)) {}

void DimMapStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DimMapStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(body);
}

bool DimMapStatementNode::visitBodies(const VisitBodyFunc& func) const {
    return func(*body);
}

DimStatementNode::DimStatementNode(std::string name, std::unique_ptr<TypeNode> type, Token token)
    : StatementNode(StatementType::kDim, std::move(token)),
      name(std::move(name)),
      type(std::move(type)),
      value(std::unique_ptr<ExpressionNode>()) {}

DimStatementNode::DimStatementNode(std::string name, std::unique_ptr<ExpressionNode> value, Token token)
    : StatementNode(StatementType::kDim, std::move(token)),
      name(std::move(name)),
      type(std::unique_ptr<TypeNode>()),
      value(std::move(value)) {}

MemberType DimStatementNode::getMemberType() const {
    return MemberType::kDimStatement;
}

void DimStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DimStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(type);
    DUMP_VAR_NODE(value);
}

std::optional<std::string> DimStatementNode::getSymbolDeclaration() const {
    return name;
}

bool DimStatementNode::visitExpressions(const VisitExpressionFunc& func) const {
    if (value) {
        if (!func(*value)) {
            return false;
        }
    }
    return true;
}

TypeNode* DimStatementNode::getChildTypeNode() const {
    if (type) {
        return type.get();
    }
    return nullptr;
}

DoConditionNode::DoConditionNode(std::unique_ptr<ExpressionNode> condition, DoConditionType conditionType, Token token)
    : Node(std::move(token)), condition(std::move(condition)), conditionType(conditionType) {}

void DoConditionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DoConditionNode);
    DUMP_VAR_NODE(condition);
    DUMP_VAR_ENUM(conditionType);
}

bool DoConditionNode::visitExpressions(const VisitExpressionFunc& func) const {
    return func(*condition);
}

DoStatementNode::DoStatementNode(
    std::unique_ptr<DoConditionNode> condition,
    DoConditionPosition conditionPosition,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(StatementType::kDo, std::move(token)),
      condition(std::move(condition)),
      conditionPosition(conditionPosition),
      body(std::move(body)) {}

void DoStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DoStatementNode);
    DUMP_VAR_NODE(condition);
    DUMP_VAR_ENUM(conditionPosition);
    DUMP_VAR_NODE(body);
}

bool DoStatementNode::visitBodies(const VisitBodyFunc& func) const {
    return func(*body);
}

bool DoStatementNode::visitExpressions(const VisitExpressionFunc& func) const {
    return condition->visitExpressions(func);
}

ExitStatementNode::ExitStatementNode(ExitScope scope, Token token)
    : StatementNode(StatementType::kExit, std::move(token)), scope(scope) {}

void ExitStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ExitStatementNode);
    DUMP_VAR_ENUM(scope);
}

ForEachStatementNode::ForEachStatementNode(
    std::string needleName,
    std::unique_ptr<ExpressionNode> haystack,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(StatementType::kForEach, std::move(token)),
      needleName(std::move(needleName)),
      haystack(std::move(haystack)),
      body(std::move(body)) {}

void ForEachStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ForEachStatementNode);
    DUMP_VAR(needleName);
    DUMP_VAR_NODE(haystack);
    DUMP_VAR_NODE(body);
}

std::optional<std::string> ForEachStatementNode::getSymbolDeclaration() const {
    return needleName;
}

bool ForEachStatementNode::visitBodies(const VisitBodyFunc& func) const {
    return func(*body);
}

bool ForEachStatementNode::visitExpressions(const VisitExpressionFunc& func) const {
    return func(*haystack);
}

ForStepNode::ForStepNode(decimal::Decimal stepImmediate, Token token)
    : Node(std::move(token)), stepImmediate(stepImmediate) {}

ForStepNode::ForStepNode(std::unique_ptr<SymbolReferenceExpressionNode> stepConstant, Token token)
    : Node(std::move(token)), stepConstant(std::move(stepConstant)) {}

void ForStepNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ForStepNode);
    DUMP_VAR_OPTIONAL_DECIMAL(stepImmediate);
    DUMP_VAR_NODE(stepConstant);
}

bool ForStepNode::visitExpressions(const VisitExpressionFunc& func) const {
    if (stepConstant) {
        if (!func(*stepConstant)) {
            return false;
        }
    }
    return true;
}

ForStatementNode::ForStatementNode(
    std::string loopVariableName,
    std::unique_ptr<ExpressionNode> fromValue,
    std::unique_ptr<ExpressionNode> toValue,
    std::unique_ptr<ForStepNode> step,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(StatementType::kFor, std::move(token)),
      loopVariableName(std::move(loopVariableName)),
      fromValue(std::move(fromValue)),
      toValue(std::move(toValue)),
      step(std::move(step)),
      body(std::move(body)) {}

void ForStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ForStatementNode);
    DUMP_VAR(loopVariableName);
    DUMP_VAR_NODE(fromValue);
    DUMP_VAR_NODE(toValue);
    DUMP_VAR_NODE(step);
    DUMP_VAR_NODE(body);
}

std::optional<std::string> ForStatementNode::getSymbolDeclaration() const {
    return loopVariableName;
}

bool ForStatementNode::visitBodies(const VisitBodyFunc& func) const {
    return func(*body);
}

bool ForStatementNode::visitExpressions(const VisitExpressionFunc& func) const {
    if (!func(*fromValue)) {
        return false;
    }
    if (!func(*toValue)) {
        return false;
    }
    if (step) {
        if (!step->visitExpressions(func)) {
            return false;
        }
    }
    return true;
}

GroupKeyNameNode::GroupKeyNameNode(std::string name, Token token) : Node(std::move(token)), name(std::move(name)) {}

void GroupKeyNameNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(GroupKeyNameNode);
    DUMP_VAR(name);
}

std::optional<std::string> GroupKeyNameNode::getSymbolDeclaration() const {
    return name;
}

GroupStatementNode::GroupStatementNode(
    std::unique_ptr<ExpressionNode> itemExpression,
    std::unique_ptr<ExpressionNode> groupingExpression,
    std::string groupName,
    std::unique_ptr<GroupKeyNameNode> keyName,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(StatementType::kGroup, std::move(token)),
      itemExpression(std::move(itemExpression)),
      groupingExpression(std::move(groupingExpression)),
      groupName(std::move(groupName)),
      keyName(std::move(keyName)),
      body(std::move(body)) {}

void GroupStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(GroupStatementNode);
    DUMP_VAR_NODE(itemExpression);
    DUMP_VAR_NODE(groupingExpression);
    DUMP_VAR(groupName);
    DUMP_VAR_NODE(keyName);
    DUMP_VAR_NODE(body);
}

std::optional<std::string> GroupStatementNode::getSymbolDeclaration() const {
    return groupName;
}

bool GroupStatementNode::visitBodies(const VisitBodyFunc& func) const {
    return func(*body);
}

bool GroupStatementNode::visitExpressions(const VisitExpressionFunc& func) const {
    if (!func(*itemExpression)) {
        return false;
    }
    if (!func(*groupingExpression)) {
        return false;
    }
    return true;
}

ElseIfNode::ElseIfNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<BodyNode> body, Token token)
    : Node(std::move(token)), condition(std::move(condition)), body(std::move(body)) {}

void ElseIfNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ElseIfNode);
    DUMP_VAR_NODE(condition);
    DUMP_VAR_NODE(body);
}

bool ElseIfNode::visitBodies(const VisitBodyFunc& func) const {
    return func(*body);
}

bool ElseIfNode::visitExpressions(const VisitExpressionFunc& func) const {
    return func(*condition);
}

IfStatementNode::IfStatementNode(
    std::unique_ptr<ExpressionNode> condition,
    std::unique_ptr<BodyNode> body,
    std::vector<std::unique_ptr<ElseIfNode>> elseIfs,
    std::unique_ptr<BodyNode> elseBody,
    Token token)
    : StatementNode(StatementType::kIf, std::move(token)),
      condition(std::move(condition)),
      body(std::move(body)),
      elseIfs(std::move(elseIfs)),
      elseBody(std::move(elseBody)) {}

void IfStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(IfStatementNode);
    DUMP_VAR_NODE(condition);
    DUMP_VAR_NODE(body);
    DUMP_VAR_NODES(elseIfs);
    DUMP_VAR_NODE(elseBody);
}

bool IfStatementNode::visitBodies(const VisitBodyFunc& func) const {
    if (!func(*body)) {
        return false;
    }
    for (const auto& x : elseIfs) {
        if (!x->visitBodies(func)) {
            return false;
        }
    }
    return true;
}

bool IfStatementNode::visitExpressions(const VisitExpressionFunc& func) const {
    if (!func(*condition)) {
        return false;
    }
    for (const auto& x : elseIfs) {
        if (!x->visitExpressions(func)) {
            return false;
        }
    }
    return true;
}

JoinStatementNode::JoinStatementNode(
    std::string needleName,
    std::unique_ptr<ExpressionNode> haystack,
    std::unique_ptr<ExpressionNode> joinExpression,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(StatementType::kJoin, std::move(token)),
      needleName(std::move(needleName)),
      haystack(std::move(haystack)),
      joinExpression(std::move(joinExpression)),
      body(std::move(body)) {}

void JoinStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(JoinStatementNode);
    DUMP_VAR(needleName);
    DUMP_VAR_NODE(haystack);
    DUMP_VAR_NODE(joinExpression);
    DUMP_VAR_NODE(body);
}

std::optional<std::string> JoinStatementNode::getSymbolDeclaration() const {
    return needleName;
}

bool JoinStatementNode::visitBodies(const VisitBodyFunc& func) const {
    return func(*body);
}

bool JoinStatementNode::visitExpressions(const VisitExpressionFunc& func) const {
    if (!func(*haystack)) {
        return false;
    }
    if (!func(*joinExpression)) {
        return false;
    }
    return true;
}

RethrowStatementNode::RethrowStatementNode(Token token) : StatementNode(StatementType::kRethrow, std::move(token)) {}

void RethrowStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(RethrowStatementNode);
}

ReturnStatementNode::ReturnStatementNode(std::unique_ptr<ExpressionNode> expression, Token token)
    : StatementNode(StatementType::kReturn, std::move(token)), expression(std::move(expression)) {}

void ReturnStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ReturnStatementNode);
    DUMP_VAR_NODE(expression);
}

bool ReturnStatementNode::visitExpressions(const VisitExpressionFunc& func) const {
    if (expression) {
        if (!func(*expression)) {
            return false;
        }
    }
    return true;
}

CaseValueNode::CaseValueNode(
    std::unique_ptr<ExpressionNode> expression,
    std::unique_ptr<ExpressionNode> toExpression,
    Token token)
    : Node(std::move(token)), expression(std::move(expression)), toExpression(std::move(toExpression)) {}

void CaseValueNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(CaseValueNode);
    DUMP_VAR_NODE(expression);
    DUMP_VAR_NODE(toExpression);
}

bool CaseValueNode::visitExpressions(const VisitExpressionFunc& func) const {
    if (!func(*expression)) {
        return false;
    }
    if (toExpression) {
        if (!func(*toExpression)) {
            return false;
        }
    }
    return true;
}

CaseNode::CaseNode(std::vector<std::unique_ptr<CaseValueNode>> values, std::unique_ptr<BodyNode> body, Token token)
    : Node(std::move(token)), values(std::move(values)), body(std::move(body)) {}

void CaseNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(CaseNode);
    DUMP_VAR_NODES(values);
    DUMP_VAR_NODE(body);
}

bool CaseNode::visitBodies(const VisitBodyFunc& func) const {
    return func(*body);
}

bool CaseNode::visitExpressions(const VisitExpressionFunc& func) const {
    for (const auto& x : values) {
        if (!x->visitExpressions(func)) {
            return false;
        }
    }
    return true;
}

SelectCaseStatementNode::SelectCaseStatementNode(
    std::unique_ptr<ExpressionNode> expression,
    std::vector<std::unique_ptr<CaseNode>> cases,
    Token token)
    : StatementNode(StatementType::kSelectCase, std::move(token)),
      expression(std::move(expression)),
      cases(std::move(cases)) {}

void SelectCaseStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(SelectCaseStatementNode);
    DUMP_VAR_NODE(expression);
    DUMP_VAR_NODES(cases);
}

bool SelectCaseStatementNode::visitBodies(const VisitBodyFunc& func) const {
    for (const auto& x : cases) {
        if (!x->visitBodies(func)) {
            return false;
        }
    }
    return true;
}

bool SelectCaseStatementNode::visitExpressions(const VisitExpressionFunc& func) const {
    return func(*expression);
}

SelectStatementNode::SelectStatementNode(
    std::unique_ptr<ExpressionNode> expression,
    std::unique_ptr<ExpressionNode> toExpression,
    Token token)
    : StatementNode(StatementType::kSelect, std::move(token)),
      expression(std::move(expression)),
      toExpression(std::move(toExpression)) {}

void SelectStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(SelectStatementNode);
    DUMP_VAR_NODE(expression);
    DUMP_VAR_NODE(toExpression);
}

bool SelectStatementNode::visitExpressions(const VisitExpressionFunc& func) const {
    if (!func(*expression)) {
        return false;
    }
    if (toExpression) {
        if (!func(*toExpression)) {
            return false;
        }
    }
    return true;
}

ThrowStatementNode::ThrowStatementNode(std::unique_ptr<ExpressionNode> expression, Token token)
    : StatementNode(StatementType::kThrow, std::move(token)), expression(std::move(expression)) {}

void ThrowStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ThrowStatementNode);
    DUMP_VAR_NODE(expression);
}

bool ThrowStatementNode::visitExpressions(const VisitExpressionFunc& func) const {
    return func(*expression);
}

TryStatementNode::TryStatementNode(
    std::unique_ptr<BodyNode> tryBody,
    std::unique_ptr<BodyNode> catchBody,
    std::unique_ptr<BodyNode> finallyBody,
    Token token)
    : StatementNode(StatementType::kTry, std::move(token)),
      tryBody(std::move(tryBody)),
      catchBody(std::move(catchBody)),
      finallyBody(std::move(finallyBody)) {}

void TryStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(TryStatementNode);
    DUMP_VAR_NODE(tryBody);
    DUMP_VAR_NODE(catchBody);
    DUMP_VAR_NODE(finallyBody);
}

bool TryStatementNode::visitBodies(const VisitBodyFunc& func) const {
    if (!func(*tryBody)) {
        return false;
    }
    if (catchBody) {
        if (!func(*catchBody)) {
            return false;
        }
    }
    if (finallyBody) {
        if (!func(*finallyBody)) {
            return false;
        }
    }
    return true;
}

WhileStatementNode::WhileStatementNode(
    std::unique_ptr<ExpressionNode> condition,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(StatementType::kWhile, std::move(token)), condition(std::move(condition)), body(std::move(body)) {}

void WhileStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(WhileStatementNode);
    DUMP_VAR_NODE(condition);
    DUMP_VAR_NODE(body);
}

bool WhileStatementNode::visitBodies(const VisitBodyFunc& func) const {
    return func(*body);
}

bool WhileStatementNode::visitExpressions(const VisitExpressionFunc& func) const {
    return func(*condition);
}

ParameterNode::ParameterNode(std::string name, std::unique_ptr<TypeNode> type, Token token)
    : Node(std::move(token)), name(std::move(name)), type(std::move(type)) {}

void ParameterNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ParameterNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(type);
}

std::optional<std::string> ParameterNode::getSymbolDeclaration() const {
    return name;
}

GlobalVariableNode::GlobalVariableNode(std::string name, bool isValue, size_t index)
    : Node(Token(0, 0, TokenKind::kError, "")), name(std::move(name)), isValue(isValue), index(index) {}

void GlobalVariableNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(GlobalVariableNode);
    DUMP_VAR(index);
}

std::optional<std::string> GlobalVariableNode::getSymbolDeclaration() const {
    return name;
}

ProcedureNode::ProcedureNode(
    std::string name,
    std::vector<std::unique_ptr<ParameterNode>> parameters,
    std::unique_ptr<TypeNode> returnType,
    std::unique_ptr<BodyNode> body,
    Token token)
    : Node(std::move(token)),
      name(std::move(name)),
      parameters(std::move(parameters)),
      returnType(std::move(returnType)),
      body(std::move(body)) {}

ProcedureNode::ProcedureNode(
    std::string name,
    std::vector<std::unique_ptr<ParameterNode>> parameters,
    std::unique_ptr<BodyNode> body,
    Token token)
    : Node(std::move(token)), name(std::move(name)), parameters(std::move(parameters)), body(std::move(body)) {}

void ProcedureNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ProcedureNode);
    DUMP_VAR(name);
    DUMP_VAR_NODES(parameters);
    DUMP_VAR_NODE(returnType);
    DUMP_VAR_NODE(body);
}

std::optional<std::string> ProcedureNode::getSymbolDeclaration() const {
    return name;
}

TypeDeclarationNode::TypeDeclarationNode(
    std::string name,
    std::vector<std::unique_ptr<ParameterNode>> fields,
    Token token)
    : Node(std::move(token)), name(std::move(name)), fields(std::move(fields)) {}

MemberType TypeDeclarationNode::getMemberType() const {
    return MemberType::kTypeDeclaration;
}

void TypeDeclarationNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(TypeDeclarationNode);
    DUMP_VAR(name);
    DUMP_VAR_NODES(fields);
}

std::optional<std::string> TypeDeclarationNode::getSymbolDeclaration() const {
    return name;
}

ProgramNode::ProgramNode(std::vector<std::unique_ptr<Node>> members, Token token)
    : Node(std::move(token)), members(std::move(members)) {}

MemberType ProcedureNode::getMemberType() const {
    return MemberType::kProcedure;
}

void ProgramNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ProgramNode);
    DUMP_VAR_NODES(members);
}

}  // namespace compiler
