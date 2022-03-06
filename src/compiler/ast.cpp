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
static void dumpVarOptional(std::ostream& s, const std::optional<T>& f, int n) {
    if (f.has_value()) {
        s << indent(n + 1) << NAMEOF(f) << '=' << *f << '\n';  // NOLINT
    }
}

// NOLINTNEXTLINE
#define DUMP_VAR_OPTIONAL(f) dumpVarOptional(s, f, n)

// NOLINTNEXTLINE
#define DUMP_VAR_DECIMAL(f) \
    { s << indent(n + 1) << NAMEOF(f) << '=' << decimalToString(f) << '\n'; }  // NOLINT

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

void Node::dump(std::ostream& /*s*/, int /*n*/) const {
    throw std::runtime_error("dump() called on node that does not implement it!");
}

MemberType Node::getMemberType() const {
    return MemberType::kNonMember;
}

std::optional<std::string> Node::getSymbolDeclaration() const {
    return std::optional<std::string>();
}

boost::local_shared_ptr<TypeNode> Node::getSymbolDeclarationType() const {
    throw std::runtime_error("getSymbolDeclarationType() called on node that does not declare a symbol!");
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

// visit all expression nodes, deep in the tree
bool Node::visitExpressions(bool /*rootsOnly*/, const VisitExpressionFunc& /*func*/) const {
    // implementations should call visitChildExpression() for their children.
    // DO NOT just call func() directly!
    return true;
}

static bool visitChildExpression(bool rootsOnly, ExpressionNode* childExpression, const VisitExpressionFunc& func) {
    if (!func(childExpression)) {
        return false;
    }
    if (rootsOnly) {
        return true;
    }
    return childExpression->visitExpressions(rootsOnly, func);
}

bool Node::isSymbolReference() const {
    return false;
}

TypeNode* Node::getChildTypeNode() const {
    return nullptr;
}

std::vector<YieldStatementNode*>* Node::getYieldStatementNodesList() {
    assert(false);
    throw std::runtime_error("This node does not support getYieldStatementNodesList.");
}

ExpressionNode::ExpressionNode(Token token) : Node(std::move(token)) {}

ConstValueExpressionNode::ConstValueExpressionNode(Token token) : ExpressionNode(std::move(token)) {}

ExpressionType ConstValueExpressionNode::getExpressionType() const {
    return ExpressionType::kConstValue;
}

StatementNode::StatementNode(Token token) : Node(std::move(token)) {}

int StatementNode::getTempLocalValueCount() const {
    return 0;
}

int StatementNode::getTempLocalObjectCount() const {
    return 0;
}

TypeNode::TypeNode(Kind kind, Token token) : Node(std::move(token)), kind(kind) {
    assert(kind != Kind::kList);
    assert(kind != Kind::kMap);
    assert(kind != Kind::kOptional);
    assert(kind != Kind::kRecord);
}

TypeNode::TypeNode(Kind kind, Token token, std::string recordName)
    : Node(std::move(token)), kind(kind), recordName(recordName) {
    assert(kind == Kind::kRecord);
}

TypeNode::TypeNode(Kind kind, Token token, boost::local_shared_ptr<TypeNode> optionalValueTypeOrListItemType)
    : Node(std::move(token)),
      kind(kind),
      listItemType(
          kind == Kind::kList ? std::move(optionalValueTypeOrListItemType) : boost::local_shared_ptr<TypeNode>()),
      optionalValueType(
          kind == Kind::kOptional ? std::move(optionalValueTypeOrListItemType) : boost::local_shared_ptr<TypeNode>()) {
    assert(kind == Kind::kOptional || kind == Kind::kList);
}

TypeNode::TypeNode(
    Kind kind,
    Token token,
    boost::local_shared_ptr<TypeNode> mapKeyType,
    boost::local_shared_ptr<TypeNode> mapValueType)
    : Node(std::move(token)), kind(kind), mapKeyType(std::move(mapKeyType)), mapValueType(std::move(mapValueType)) {
    assert(kind == Kind::kMap);
}

TypeNode::TypeNode(Kind kind, Token token, std::vector<boost::local_shared_ptr<ParameterNode>> aFields)
    : Node(std::move(token)), kind(kind), fields(std::move(aFields)) {
    assert(kind == Kind::kRecord);

    auto valueIndex = 0;
    auto objectIndex = 0;
    for (auto& field : fields) {
        if (field->type->isValueType()) {
            field->fieldValueIndex = valueIndex++;
        } else {
            field->fieldObjectIndex = objectIndex++;
        }
    }
}

void TypeNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(TypeNode);
    DUMP_VAR_ENUM(kind);
    DUMP_VAR_OPTIONAL(recordName);
    DUMP_VAR_NODES(fields);
    DUMP_VAR_NODE(listItemType);
    DUMP_VAR_NODE(mapKeyType);
    DUMP_VAR_NODE(mapValueType);
    DUMP_VAR_NODE(optionalValueType);
}

bool TypeNode::isValueType() const {
    return kind == Kind::kNumber || kind == Kind::kBoolean || kind == Kind::kDate || kind == Kind::kDateTime ||
        kind == Kind::kTimeSpan;
}

bool TypeNode::equals(const TypeNode& target) const {
    if (kind != target.kind) {
        return false;
    }

    switch (kind) {
        case Kind::kAny:
        case Kind::kBoolean:
        case Kind::kNumber:
        case Kind::kDate:
        case Kind::kDateTime:
        case Kind::kDateTimeOffset:
        case Kind::kTimeSpan:
        case Kind::kTimeZone:
        case Kind::kString:
            return true;

        case Kind::kList:
            assert(target.listItemType != nullptr);
            return listItemType->equals(*target.listItemType);

        case Kind::kMap:
            return mapKeyType->equals(*target.mapKeyType) && mapValueType->equals(*target.mapValueType);

        case Kind::kRecord:
            if (recordName.has_value() != target.recordName.has_value()) {
                return false;
            }
            if (fields.size() != target.fields.size()) {
                return false;
            }
            for (size_t i = 0; i < fields.size(); i++) {
                const auto& srcField = *fields[i];
                const auto& dstField = *target.fields[i];
                if (srcField.name != dstField.name || !srcField.type->equals(*dstField.type)) {
                    return false;
                }
            }
            return true;

        case Kind::kOptional:
            assert(target.optionalValueType != nullptr);
            return optionalValueType->equals(*target.optionalValueType);

        default:
            throw std::runtime_error(fmt::format("Unhandled kind {}", NAMEOF_ENUM(kind)));
    }
}

std::string TypeNode::toString() const {
    switch (kind) {
        case Kind::kBoolean:
            return "Boolean";
        case Kind::kNumber:
            return "Number";
        case Kind::kDate:
            return "Date";
        case Kind::kDateTime:
            return "DateTime";
        case Kind::kDateTimeOffset:
            return "DateTimeOffset";
        case Kind::kTimeSpan:
            return "TimeSpan";
        case Kind::kTimeZone:
            return "TimeZone";
        case Kind::kString:
            return "String";
        case Kind::kList:
            return fmt::format("List of {}", listItemType->toString());
        case Kind::kMap:
            return fmt::format("Map from {} to {}", mapKeyType->toString(), mapValueType->toString());
        case Kind::kRecord:
            if (recordName.has_value()) {
                return *recordName;
            } else {
                std::ostringstream ss;
                ss << "Record (";
                for (size_t i = 0; i < fields.size(); i++) {
                    if (i > 0) {
                        ss << ", ";
                    }
                    ss << fields[i]->name << " as " << fields[i]->type->toString();
                }
                ss << ")";
                return ss.str();
            }
        case Kind::kOptional:
            return fmt::format("Optional {}", optionalValueType->toString());
        default:
            throw std::runtime_error("not impl");
    }
}

BinaryExpressionSuffixNode::BinaryExpressionSuffixNode(
    BinaryOperator binaryOperator,
    std::unique_ptr<ExpressionNode> rightOperand,
    Token token)
    : Node(std::move(token)), binaryOperator(binaryOperator), rightOperand(std::move(rightOperand)) {}

void BinaryExpressionSuffixNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(BinaryExpressionSuffixNode);
    DUMP_VAR_ENUM(binaryOperator);
    DUMP_VAR_NODE(rightOperand);
}

bool BinaryExpressionSuffixNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    return visitChildExpression(rootsOnly, rightOperand.get(), func);
}

BinaryExpressionNode::BinaryExpressionNode(
    std::unique_ptr<ExpressionNode> leftOperand,
    std::vector<std::unique_ptr<BinaryExpressionSuffixNode>> binarySuffixes,
    Token token)
    : ExpressionNode(std::move(token)),
      leftOperand(std::move(leftOperand)),
      binarySuffixes(std::move(binarySuffixes)) {}

void BinaryExpressionNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(BinaryExpressionNode);
    DUMP_VAR_NODE(leftOperand);
    DUMP_VAR_NODES(binarySuffixes);
}

bool BinaryExpressionNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    if (!visitChildExpression(rootsOnly, leftOperand.get(), func)) {
        return false;
    }
    for (const auto& x : binarySuffixes) {
        if (!x->visitExpressions(rootsOnly, func)) {
            return false;
        }
    }
    return true;
}

ExpressionType BinaryExpressionNode::getExpressionType() const {
    return ExpressionType::kBinary;
}

ConvertExpressionNode::ConvertExpressionNode(
    std::unique_ptr<ExpressionNode> value,
    boost::local_shared_ptr<TypeNode> type,
    Token token)
    : ExpressionNode(std::move(token)), value(std::move(value)), type(std::move(type)) {}

void ConvertExpressionNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(ConvertExpressionNode);
    DUMP_VAR_NODE(value);
    DUMP_VAR_NODE(type);
}

bool ConvertExpressionNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    return visitChildExpression(rootsOnly, value.get(), func);
}

TypeNode* ConvertExpressionNode::getChildTypeNode() const {
    return type.get();
}

ExpressionType ConvertExpressionNode::getExpressionType() const {
    return ExpressionType::kConvert;
}

DottedExpressionSuffixNode::DottedExpressionSuffixNode(std::string aName, Token token)
    : Node(std::move(token)), name(std::move(aName)), nameLowercase(boost::to_lower_copy(*name)) {}

DottedExpressionSuffixNode::DottedExpressionSuffixNode(
    std::vector<std::unique_ptr<ExpressionNode>> collectionIndexOrCallArgs,
    Token token)
    : Node(std::move(token)), collectionIndexOrCallArgs(std::move(collectionIndexOrCallArgs)) {}

void DottedExpressionSuffixNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(DottedExpressionSuffixNode);
    if (name.has_value()) {
        auto dottedName = fmt::format(".{}", *name);
        DUMP_VAR(dottedName);
    }
    for (auto& collectionIndexOrCallArg : collectionIndexOrCallArgs) {
        DUMP_VAR_NODE(collectionIndexOrCallArg);
    }
}

bool DottedExpressionSuffixNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    for (auto& x : collectionIndexOrCallArgs) {
        if (!visitChildExpression(rootsOnly, x.get(), func)) {
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

void DottedExpressionNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(DottedExpressionNode);
    DUMP_VAR_NODE(base);
    DUMP_VAR_NODES(dottedSuffixes);
}

bool DottedExpressionNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    if (!visitChildExpression(rootsOnly, base.get(), func)) {
        return false;
    }
    for (const auto& x : dottedSuffixes) {
        if (!x->visitExpressions(rootsOnly, func)) {
            return false;
        }
    }
    return true;
}

ExpressionType DottedExpressionNode::getExpressionType() const {
    return ExpressionType::kDotted;
}

FunctionCallExpressionNode::FunctionCallExpressionNode(
    std::string name,
    std::vector<std::unique_ptr<ExpressionNode>> args,
    Token token)
    : ExpressionNode(std::move(token)), name(std::move(name)), args(std::move(args)) {}

void FunctionCallExpressionNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(FunctionCallExpressionNode);
    DUMP_VAR(name);
    DUMP_VAR_NODES(args);
}

bool FunctionCallExpressionNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    for (auto& x : args) {
        if (!visitChildExpression(rootsOnly, x.get(), func)) {
            return false;
        }
    }
    return true;
}

ExpressionType FunctionCallExpressionNode::getExpressionType() const {
    return ExpressionType::kFunctionCall;
}

LiteralArrayExpressionNode::LiteralArrayExpressionNode(
    std::vector<std::unique_ptr<ExpressionNode>> elements,
    Token token)
    : ConstValueExpressionNode(std::move(token)), elements(std::move(elements)) {}

void LiteralArrayExpressionNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(LiteralArrayExpressionNode);
    DUMP_VAR_NODES(elements);
}

bool LiteralArrayExpressionNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    for (const auto& x : elements) {
        if (!visitChildExpression(rootsOnly, x.get(), func)) {
            return false;
        }
    }
    return true;
}

ConstValueExpressionType LiteralArrayExpressionNode::getConstValueExpressionType() const {
    return ConstValueExpressionType::kArray;
}

LiteralBooleanExpressionNode::LiteralBooleanExpressionNode(bool value, Token token)
    : ConstValueExpressionNode(std::move(token)), value(value) {}

void LiteralBooleanExpressionNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(LiteralBooleanExpressionNode);
    DUMP_VAR(value);
}

ConstValueExpressionType LiteralBooleanExpressionNode::getConstValueExpressionType() const {
    return ConstValueExpressionType::kBoolean;
}

LiteralNumberExpressionNode::LiteralNumberExpressionNode(decimal::Decimal value, Token token)
    : ConstValueExpressionNode(std::move(token)), value(std::move(value)) {}

void LiteralNumberExpressionNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(LiteralNumberExpressionNode);
    DUMP_VAR_DECIMAL(value);
}

ConstValueExpressionType LiteralNumberExpressionNode::getConstValueExpressionType() const {
    return ConstValueExpressionType::kNumber;
}

LiteralRecordFieldNode::LiteralRecordFieldNode(std::string key, std::unique_ptr<ExpressionNode> value, Token token)
    : Node(std::move(token)), key(std::move(key)), value(std::move(value)) {}

void LiteralRecordFieldNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(LiteralRecordFieldNode);
    DUMP_VAR(key);
    DUMP_VAR_NODE(value);
}

bool LiteralRecordFieldNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    return visitChildExpression(rootsOnly, value.get(), func);
}

LiteralRecordExpressionNode::LiteralRecordExpressionNode(
    std::vector<std::unique_ptr<LiteralRecordFieldNode>> fields,
    Token token)
    : ConstValueExpressionNode(std::move(token)), fields(std::move(fields)) {}

void LiteralRecordExpressionNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(LiteralRecordExpressionNode);
    DUMP_VAR_NODES(fields);
}

bool LiteralRecordExpressionNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    for (const auto& x : fields) {
        if (!x->visitExpressions(rootsOnly, func)) {
            return false;
        }
    }
    return true;
}

ConstValueExpressionType LiteralRecordExpressionNode::getConstValueExpressionType() const {
    return ConstValueExpressionType::kRecord;
}

LiteralStringExpressionNode::LiteralStringExpressionNode(std::string value, Token token)
    : ConstValueExpressionNode(std::move(token)), value(std::move(value)) {}

void LiteralStringExpressionNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(LiteralStringExpressionNode);
    DUMP_VAR(value);
}

ConstValueExpressionType LiteralStringExpressionNode::getConstValueExpressionType() const {
    return ConstValueExpressionType::kString;
}

NotExpressionNode::NotExpressionNode(std::unique_ptr<ExpressionNode> operand, Token token)
    : ExpressionNode(std::move(token)), operand(std::move(operand)) {}

void NotExpressionNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(NotExpressionNode);
    DUMP_VAR_NODE(operand);
}

bool NotExpressionNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    return visitChildExpression(rootsOnly, operand.get(), func);
}

ExpressionType NotExpressionNode::getExpressionType() const {
    return ExpressionType::kNot;
}

SymbolReferenceExpressionNode::SymbolReferenceExpressionNode(std::string name, Token token)
    : ExpressionNode(std::move(token)), name(std::move(name)) {}

void SymbolReferenceExpressionNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(SymbolReferenceExpressionNode);
    DUMP_VAR(name);
}

bool SymbolReferenceExpressionNode::isSymbolReference() const {
    return true;
}

ExpressionType SymbolReferenceExpressionNode::getExpressionType() const {
    return ExpressionType::kSymbolReference;
}

AssignStatementNode::AssignStatementNode(
    std::unique_ptr<ExpressionNode> target,
    std::unique_ptr<ExpressionNode> value,
    Token token)
    : StatementNode(std::move(token)), target(std::move(target)), value(std::move(value)) {}

void AssignStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(AssignStatementNode);
    DUMP_VAR_NODE(target);
    DUMP_VAR_NODE(value);
}

bool AssignStatementNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    if (!visitChildExpression(rootsOnly, target.get(), func)) {
        return false;
    }
    return visitChildExpression(rootsOnly, value.get(), func);
}

StatementType AssignStatementNode::getStatementType() const {
    return StatementType::kAssign;
}

BodyNode::BodyNode(std::vector<std::unique_ptr<StatementNode>> statements, Token token)
    : Node(std::move(token)), statements(std::move(statements)) {}

void BodyNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(BodyNode);
    DUMP_VAR_NODES(statements);
}

CallStatementNode::CallStatementNode(
    std::string name,
    std::vector<std::unique_ptr<ExpressionNode>> arguments,
    Token token)
    : StatementNode(std::move(token)), name(std::move(name)), arguments(std::move(arguments)) {}

void CallStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(CallStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODES(arguments);
}

bool CallStatementNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    for (const auto& x : arguments) {
        if (!visitChildExpression(rootsOnly, x.get(), func)) {
            return false;
        }
    }
    return true;
}

StatementType CallStatementNode::getStatementType() const {
    return StatementType::kCall;
}

ConstStatementNode::ConstStatementNode(std::string name, std::unique_ptr<ConstValueExpressionNode> value, Token token)
    : StatementNode(std::move(token)), name(std::move(name)), value(std::move(value)) {}

MemberType ConstStatementNode::getMemberType() const {
    return MemberType::kConstStatement;
}

void ConstStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(ConstStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(value);
}

std::optional<std::string> ConstStatementNode::getSymbolDeclaration() const {
    return name;
}

StatementType ConstStatementNode::getStatementType() const {
    return StatementType::kConst;
}

boost::local_shared_ptr<TypeNode> ConstStatementNode::getSymbolDeclarationType() const {
    return evaluatedType;
}

bool ConstStatementNode::isSymbolVisibleToSiblingStatements() const {
    return true;
}

bool ConstStatementNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    if (value) {
        if (!visitChildExpression(rootsOnly, value.get(), func)) {
            return false;
        }
    }
    return true;
}

ContinueStatementNode::ContinueStatementNode(LoopType scope, Token token)
    : StatementNode(std::move(token)), scope(scope) {}

void ContinueStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(ContinueStatementNode);
    DUMP_VAR_ENUM(scope);
}

StatementType ContinueStatementNode::getStatementType() const {
    return StatementType::kContinue;
}

DimListStatementNode::DimListStatementNode(std::string name, std::unique_ptr<BodyNode> body, Token token)
    : StatementNode(std::move(token)), name(std::move(name)), body(std::move(body)) {}

void DimListStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(DimListStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(body);
}

bool DimListStatementNode::visitBodies(const VisitBodyFunc& func) const {
    return func(body.get());
}

StatementType DimListStatementNode::getStatementType() const {
    return StatementType::kDimList;
}

std::optional<std::string> DimListStatementNode::getSymbolDeclaration() const {
    return name;
}

boost::local_shared_ptr<TypeNode> DimListStatementNode::getSymbolDeclarationType() const {
    assert(evaluatedType != nullptr);
    return evaluatedType;
}

bool DimListStatementNode::isSymbolVisibleToSiblingStatements() const {
    return true;
}

std::vector<YieldStatementNode*>* DimListStatementNode::getYieldStatementNodesList() {
    return &yieldStatements;
}

DimMapStatementNode::DimMapStatementNode(std::string name, std::unique_ptr<BodyNode> body, Token token)
    : StatementNode(std::move(token)), name(std::move(name)), body(std::move(body)) {}

void DimMapStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(DimMapStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(body);
}

bool DimMapStatementNode::visitBodies(const VisitBodyFunc& func) const {
    return func(body.get());
}

StatementType DimMapStatementNode::getStatementType() const {
    return StatementType::kDimMap;
}

std::optional<std::string> DimMapStatementNode::getSymbolDeclaration() const {
    return name;
}

boost::local_shared_ptr<TypeNode> DimMapStatementNode::getSymbolDeclarationType() const {
    assert(evaluatedType != nullptr);
    return evaluatedType;
}

bool DimMapStatementNode::isSymbolVisibleToSiblingStatements() const {
    return true;
}

std::vector<YieldStatementNode*>* DimMapStatementNode::getYieldStatementNodesList() {
    return &yieldStatements;
}

DimStatementNode::DimStatementNode(std::string name, boost::local_shared_ptr<TypeNode> type, Token token, bool shared)
    : StatementNode(std::move(token)),
      name(std::move(name)),
      type(std::move(type)),
      value(std::unique_ptr<ExpressionNode>()),
      shared(shared) {}

DimStatementNode::DimStatementNode(std::string name, std::unique_ptr<ExpressionNode> value, Token token, bool shared)
    : StatementNode(std::move(token)), name(std::move(name)), value(std::move(value)), shared(shared) {}

MemberType DimStatementNode::getMemberType() const {
    return MemberType::kDimStatement;
}

void DimStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(DimStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(type);
    DUMP_VAR_NODE(value);
}

std::optional<std::string> DimStatementNode::getSymbolDeclaration() const {
    return name;
}

bool DimStatementNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    if (value) {
        if (!visitChildExpression(rootsOnly, value.get(), func)) {
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

StatementType DimStatementNode::getStatementType() const {
    return StatementType::kDim;
}

boost::local_shared_ptr<TypeNode> DimStatementNode::getSymbolDeclarationType() const {
    return evaluatedType;
}

bool DimStatementNode::isSymbolVisibleToSiblingStatements() const {
    return true;
}

DoStatementNode::DoStatementNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<BodyNode> body, Token token)
    : StatementNode(std::move(token)), condition(std::move(condition)), body(std::move(body)) {}

void DoStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(DoStatementNode);
    DUMP_VAR_NODE(condition);
    DUMP_VAR_NODE(body);
}

bool DoStatementNode::visitBodies(const VisitBodyFunc& func) const {
    return func(body.get());
}

bool DoStatementNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    return visitChildExpression(rootsOnly, condition.get(), func);
}

StatementType DoStatementNode::getStatementType() const {
    return StatementType::kDo;
}

ExitStatementNode::ExitStatementNode(LoopType scope, Token token) : StatementNode(std::move(token)), scope(scope) {}

void ExitStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(ExitStatementNode);
    DUMP_VAR_ENUM(scope);
}

StatementType ExitStatementNode::getStatementType() const {
    return StatementType::kExit;
}

ForEachStatementNode::ForEachStatementNode(
    std::string needleName,
    std::unique_ptr<ExpressionNode> haystack,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(std::move(token)),
      needleName(std::move(needleName)),
      haystack(std::move(haystack)),
      body(std::move(body)) {}

void ForEachStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(ForEachStatementNode);
    DUMP_VAR(needleName);
    DUMP_VAR_NODE(haystack);
    DUMP_VAR_NODE(body);
}

std::optional<std::string> ForEachStatementNode::getSymbolDeclaration() const {
    return needleName;
}

bool ForEachStatementNode::visitBodies(const VisitBodyFunc& func) const {
    return func(body.get());
}

bool ForEachStatementNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    return visitChildExpression(rootsOnly, haystack.get(), func);
}

StatementType ForEachStatementNode::getStatementType() const {
    return StatementType::kForEach;
}

boost::local_shared_ptr<TypeNode> ForEachStatementNode::getSymbolDeclarationType() const {
    auto listType = haystack->evaluatedType;
    assert(listType->listItemType);
    return listType->listItemType;
}

int ForEachStatementNode::getTempLocalValueCount() const {
    return 2;
}

int ForEachStatementNode::getTempLocalObjectCount() const {
    return 1;
}

ForStatementNode::ForStatementNode(
    std::string loopVariableName,
    std::unique_ptr<ExpressionNode> fromValue,
    std::unique_ptr<ExpressionNode> toValue,
    std::unique_ptr<ExpressionNode> step,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(std::move(token)),
      loopVariableName(std::move(loopVariableName)),
      fromValue(std::move(fromValue)),
      toValue(std::move(toValue)),
      step(std::move(step)),
      body(std::move(body)) {}

void ForStatementNode::dump(std::ostream& s, int n) const {
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
    return func(body.get());
}

bool ForStatementNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    if (!visitChildExpression(rootsOnly, fromValue.get(), func)) {
        return false;
    }
    if (!visitChildExpression(rootsOnly, toValue.get(), func)) {
        return false;
    }
    if (step != nullptr) {
        if (!visitChildExpression(rootsOnly, step.get(), func)) {
            return false;
        }
    }
    return true;
}

StatementType ForStatementNode::getStatementType() const {
    return StatementType::kFor;
}

boost::local_shared_ptr<TypeNode> ForStatementNode::getSymbolDeclarationType() const {
    if (!_type) {
        _type = boost::make_local_shared<TypeNode>(Kind::kNumber, token);
    }
    return _type;
}

int ForStatementNode::getTempLocalValueCount() const {
    // one for the max value
    // one for the step value
    return 2;
}

ElseIfNode::ElseIfNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<BodyNode> body, Token token)
    : Node(std::move(token)), condition(std::move(condition)), body(std::move(body)) {}

void ElseIfNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(ElseIfNode);
    DUMP_VAR_NODE(condition);
    DUMP_VAR_NODE(body);
}

bool ElseIfNode::visitBodies(const VisitBodyFunc& func) const {
    return func(body.get());
}

bool ElseIfNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    return visitChildExpression(rootsOnly, condition.get(), func);
}

IfStatementNode::IfStatementNode(
    std::unique_ptr<ExpressionNode> condition,
    std::unique_ptr<BodyNode> body,
    std::vector<std::unique_ptr<ElseIfNode>> elseIfs,
    std::unique_ptr<BodyNode> elseBody,
    Token token)
    : StatementNode(std::move(token)),
      condition(std::move(condition)),
      body(std::move(body)),
      elseIfs(std::move(elseIfs)),
      elseBody(std::move(elseBody)) {}

void IfStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(IfStatementNode);
    DUMP_VAR_NODE(condition);
    DUMP_VAR_NODE(body);
    DUMP_VAR_NODES(elseIfs);
    DUMP_VAR_NODE(elseBody);
}

bool IfStatementNode::visitBodies(const VisitBodyFunc& func) const {
    if (!func(body.get())) {
        return false;
    }
    if (elseBody != nullptr && !func(elseBody.get())) {
        return false;
    }
    for (const auto& x : elseIfs) {
        if (!x->visitBodies(func)) {
            return false;
        }
    }
    return true;
}

bool IfStatementNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    if (!visitChildExpression(rootsOnly, condition.get(), func)) {
        return false;
    }
    for (const auto& x : elseIfs) {
        if (!x->visitExpressions(rootsOnly, func)) {
            return false;
        }
    }
    return true;
}

StatementType IfStatementNode::getStatementType() const {
    return StatementType::kIf;
}

RethrowStatementNode::RethrowStatementNode(Token token) : StatementNode(std::move(token)) {}

void RethrowStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(RethrowStatementNode);
}

StatementType RethrowStatementNode::getStatementType() const {
    return StatementType::kRethrow;
}

ReturnStatementNode::ReturnStatementNode(std::unique_ptr<ExpressionNode> expression, Token token)
    : StatementNode(std::move(token)), expression(std::move(expression)) {}

void ReturnStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(ReturnStatementNode);
    DUMP_VAR_NODE(expression);
}

bool ReturnStatementNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    if (expression) {
        if (!visitChildExpression(rootsOnly, expression.get(), func)) {
            return false;
        }
    }
    return true;
}

StatementType ReturnStatementNode::getStatementType() const {
    return StatementType::kReturn;
}

CaseValueNode::CaseValueNode(
    std::unique_ptr<ExpressionNode> expression,
    std::unique_ptr<ExpressionNode> toExpression,
    Token token)
    : Node(std::move(token)), expression(std::move(expression)), toExpression(std::move(toExpression)) {}

void CaseValueNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(CaseValueNode);
    DUMP_VAR_NODE(expression);
    DUMP_VAR_NODE(toExpression);
}

bool CaseValueNode::visitExpressions(bool /*rootsOnly*/, const VisitExpressionFunc& func) const {
    if (!func(expression.get())) {
        return false;
    }
    if (toExpression) {
        if (!func(toExpression.get())) {
            return false;
        }
    }
    return true;
}

CaseNode::CaseNode(std::vector<std::unique_ptr<CaseValueNode>> values, std::unique_ptr<BodyNode> body, Token token)
    : Node(std::move(token)), values(std::move(values)), body(std::move(body)) {}

void CaseNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(CaseNode);
    DUMP_VAR_NODES(values);
    DUMP_VAR_NODE(body);
}

bool CaseNode::visitBodies(const VisitBodyFunc& func) const {
    return func(body.get());
}

bool CaseNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    for (const auto& x : values) {
        if (!x->visitExpressions(rootsOnly, func)) {
            return false;
        }
    }
    return true;
}

SelectCaseStatementNode::SelectCaseStatementNode(
    std::unique_ptr<ExpressionNode> expression,
    std::vector<std::unique_ptr<CaseNode>> cases,
    Token token)
    : StatementNode(std::move(token)), expression(std::move(expression)), cases(std::move(cases)) {}

void SelectCaseStatementNode::dump(std::ostream& s, int n) const {
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

bool SelectCaseStatementNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    if (!visitChildExpression(rootsOnly, expression.get(), func)) {
        return false;
    }
    for (const auto& x : cases) {
        if (!x->visitExpressions(rootsOnly, func)) {
            return false;
        }
    }
    return true;
}

StatementType SelectCaseStatementNode::getStatementType() const {
    return StatementType::kSelectCase;
}

int SelectCaseStatementNode::getTempLocalValueCount() const {
    return 1;
}

int SelectCaseStatementNode::getTempLocalObjectCount() const {
    return 1;
}

YieldStatementNode::YieldStatementNode(
    std::unique_ptr<ExpressionNode> expression,
    std::unique_ptr<ExpressionNode> toExpression,
    Token token)
    : StatementNode(std::move(token)), expression(std::move(expression)), toExpression(std::move(toExpression)) {}

void YieldStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(YieldStatementNode);
    DUMP_VAR_NODE(expression);
    DUMP_VAR_NODE(toExpression);
}

bool YieldStatementNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    if (!visitChildExpression(rootsOnly, expression.get(), func)) {
        return false;
    }
    if (toExpression) {
        if (!visitChildExpression(rootsOnly, toExpression.get(), func)) {
            return false;
        }
    }
    return true;
}

StatementType YieldStatementNode::getStatementType() const {
    return StatementType::kYield;
}

ThrowStatementNode::ThrowStatementNode(
    std::unique_ptr<ExpressionNode> message,
    std::unique_ptr<ExpressionNode> code,
    Token token)
    : StatementNode(std::move(token)), message(std::move(message)), code(std::move(code)) {}

void ThrowStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(ThrowStatementNode);
    if (code != nullptr) {
        DUMP_VAR_NODE(code);
    }
    DUMP_VAR_NODE(message);
}

bool ThrowStatementNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    if (code != nullptr && !visitChildExpression(rootsOnly, code.get(), func)) {
        return false;
    }
    if (!visitChildExpression(rootsOnly, message.get(), func)) {
        return false;
    }
    return true;
}

StatementType ThrowStatementNode::getStatementType() const {
    return StatementType::kThrow;
}

TryStatementNode::TryStatementNode(std::unique_ptr<BodyNode> tryBody, std::unique_ptr<BodyNode> catchBody, Token token)
    : StatementNode(std::move(token)), tryBody(std::move(tryBody)), catchBody(std::move(catchBody)) {}

void TryStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(TryStatementNode);
    DUMP_VAR_NODE(tryBody);
    DUMP_VAR_NODE(catchBody);
}

bool TryStatementNode::visitBodies(const VisitBodyFunc& func) const {
    if (!func(tryBody.get())) {
        return false;
    }
    if (catchBody) {
        if (!func(catchBody.get())) {
            return false;
        }
    }
    return true;
}

StatementType TryStatementNode::getStatementType() const {
    return StatementType::kTry;
}

WhileStatementNode::WhileStatementNode(
    std::unique_ptr<ExpressionNode> condition,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(std::move(token)), condition(std::move(condition)), body(std::move(body)) {}

void WhileStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(WhileStatementNode);
    DUMP_VAR_NODE(condition);
    DUMP_VAR_NODE(body);
}

bool WhileStatementNode::visitBodies(const VisitBodyFunc& func) const {
    return func(body.get());
}

bool WhileStatementNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    return visitChildExpression(rootsOnly, condition.get(), func);
}

StatementType WhileStatementNode::getStatementType() const {
    return StatementType::kWhile;
}

PrintStatementNode::PrintStatementNode(
    std::vector<std::unique_ptr<ExpressionNode>> expressions,
    bool trailingSemicolon,
    Token token)
    : StatementNode(std::move(token)), expressions(std::move(expressions)), trailingSemicolon(trailingSemicolon) {}

void PrintStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(PrintStatementNode);
    DUMP_VAR_NODES(expressions);
    DUMP_VAR(trailingSemicolon);
}

bool PrintStatementNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    for (const auto& x : expressions) {
        if (!visitChildExpression(rootsOnly, x.get(), func)) {
            return false;
        }
    }
    return true;
}

StatementType PrintStatementNode::getStatementType() const {
    return StatementType::kPrint;
}

InputStatementNode::InputStatementNode(std::unique_ptr<ExpressionNode> target, Token token)
    : StatementNode(std::move(token)), target(std::move(target)) {}

void InputStatementNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(InputStatementNode);
    DUMP_VAR_NODE(target);
}

bool InputStatementNode::visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const {
    return visitChildExpression(rootsOnly, target.get(), func);
}

StatementType InputStatementNode::getStatementType() const {
    return StatementType::kInput;
}

ParameterNode::ParameterNode(std::string aName, boost::local_shared_ptr<TypeNode> type, Token token)
    : Node(std::move(token)),
      name(std::move(aName)),
      nameLowercase(boost::to_lower_copy(name)),
      type(std::move(type)) {}

void ParameterNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(ParameterNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(type);
}

std::optional<std::string> ParameterNode::getSymbolDeclaration() const {
    return name;
}

boost::local_shared_ptr<TypeNode> ParameterNode::getSymbolDeclarationType() const {
    return type;
}

GlobalVariableNode::GlobalVariableNode(std::string name)
    : Node(Token(0, 0, TokenKind::kError, "", nullptr)), name(std::move(std::move(name))) {}

void GlobalVariableNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(GlobalVariableNode);
}

std::optional<std::string> GlobalVariableNode::getSymbolDeclaration() const {
    return name;
}

boost::local_shared_ptr<TypeNode> GlobalVariableNode::getSymbolDeclarationType() const {
    return evaluatedType;
}

ProcedureNode::ProcedureNode(
    std::string name,
    std::vector<std::unique_ptr<ParameterNode>> parameters,
    boost::local_shared_ptr<TypeNode> returnType,
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

void ProcedureNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(ProcedureNode);
    DUMP_VAR(name);
    DUMP_VAR_NODES(parameters);
    DUMP_VAR_NODE(returnType);
    DUMP_VAR_NODE(body);
}

MemberType ProcedureNode::getMemberType() const {
    return MemberType::kProcedure;
}

std::optional<std::string> ProcedureNode::getSymbolDeclaration() const {
    return name;
}

std::vector<YieldStatementNode*>* ProcedureNode::getYieldStatementNodesList() {
    return &yieldStatements;
}

TypeDeclarationNode::TypeDeclarationNode(
    std::string name,
    std::vector<std::unique_ptr<ParameterNode>> fields,
    Token token)
    : Node(std::move(token)), name(std::move(name)), fields(std::move(fields)) {}

MemberType TypeDeclarationNode::getMemberType() const {
    return MemberType::kTypeDeclaration;
}

void TypeDeclarationNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(TypeDeclarationNode);
    DUMP_VAR(name);
    DUMP_VAR_NODES(fields);
}

ProgramNode::ProgramNode(std::vector<std::unique_ptr<Node>> members, Token token)
    : Node(std::move(token)), members(std::move(members)) {}

void ProgramNode::dump(std::ostream& s, int n) const {
    DUMP_TYPE(ProgramNode);
    DUMP_VAR_NODES(members);
}

}  // namespace compiler
