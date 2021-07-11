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

ExpressionType ConstValueExpressionNode::getExpressionType() const {
    return ExpressionType::kConstValue;
}

StatementNode::StatementNode(Token token) : Node(std::move(token)) {}

TypeNode::TypeNode(Kind kind, Token token) : Node(std::move(token)), kind(kind) {}

TypeNode::TypeNode(Kind kind, Token token, std::string recordName)
    : Node(std::move(token)), kind(kind), recordName(recordName) {}

TypeNode::TypeNode(Kind kind, Token token, boost::local_shared_ptr<TypeNode> optionalValueTypeOrListItemType)
    : Node(std::move(token)),
      kind(kind),
      listItemType(
          kind == Kind::kList ? std::move(optionalValueTypeOrListItemType) : boost::local_shared_ptr<TypeNode>()),
      optionalValueType(
          kind == Kind::kOptional ? std::move(optionalValueTypeOrListItemType) : boost::local_shared_ptr<TypeNode>()) {}

TypeNode::TypeNode(
    Kind kind,
    Token token,
    boost::local_shared_ptr<TypeNode> mapKeyType,
    boost::local_shared_ptr<TypeNode> mapValueType)
    : Node(std::move(token)), kind(kind), mapKeyType(std::move(mapKeyType)), mapValueType(std::move(mapValueType)) {}

TypeNode::TypeNode(Kind kind, Token token, std::vector<boost::local_shared_ptr<ParameterNode>> fields)
    : Node(std::move(token)), kind(kind), fields(std::move(fields)) {}

static std::vector<boost::local_shared_ptr<ParameterNode>> cloneFields(
    const std::vector<boost::local_shared_ptr<ParameterNode>>& source) {
    std::vector<boost::local_shared_ptr<ParameterNode>> dest;
    dest.reserve(source.size());
    for (const auto& n : source) {
        dest.push_back(boost::make_local_shared<ParameterNode>(*n));
    }
    return dest;
}

static boost::local_shared_ptr<TypeNode> cloneType(const boost::local_shared_ptr<TypeNode>& source) {
    if (source) {
        return boost::make_local_shared<TypeNode>(*source);
    }
    return nullptr;
}

TypeNode::TypeNode(const TypeNode& source)
    : Node(source.token),
      kind(source.kind),
      recordName(source.recordName),
      genericPlaceholderName(source.genericPlaceholderName),
      fields(cloneFields(source.fields)),
      listItemType(cloneType(source.listItemType)),
      mapKeyType(cloneType(source.mapKeyType)),
      mapValueType(cloneType(source.mapValueType)),
      optionalValueType(cloneType(source.optionalValueType)) {}

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

bool TypeNode::isValueType() const {
    return kind == Kind::kNumber || kind == Kind::kBoolean || kind == Kind::kDate || kind == Kind::kDateTime ||
        kind == Kind::kTimeSpan;
}

bool TypeNode::canImplicitlyConvertTo(const TypeNode& target) const {
    if (target.kind == Kind::kOptional) {
        return canImplicitlyConvertTo(*target.optionalValueType);
    }

    if (kind != target.kind) {
        return false;
    }

    switch (kind) {
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
            return listItemType->canImplicitlyConvertTo(*target.listItemType);

        case Kind::kMap:
            return mapKeyType->canImplicitlyConvertTo(*target.mapKeyType) &&
                mapValueType->canImplicitlyConvertTo(*target.mapValueType);

        case Kind::kRecord:
            if (!recordName.has_value() && target.recordName.has_value()) {
                return false;  // can't convert anonymous -> named type
            }
            if (recordName.has_value() && target.recordName.has_value()) {
                return *recordName == *target.recordName;  // named -> named has to be the identical record name
            }
            // named -> anonymous or anonymous -> anonymous has to have matching fields
            if (fields.size() != target.fields.size()) {
                return false;
            }
            for (size_t i = 0; i < fields.size(); i++) {
                const auto& srcField = *fields[i];
                const auto& dstField = *target.fields[i];
                if (srcField.name != dstField.name || !srcField.type->isIdentical(*dstField.type)) {
                    return false;
                }
            }
            return true;

        default:
            throw std::runtime_error("not impl");
    }
}

bool TypeNode::isIdentical(const TypeNode& target) const {
    if (kind != target.kind) {
        return false;
    }

    switch (kind) {
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
            return listItemType->isIdentical(*target.listItemType);

        case Kind::kMap:
            return mapKeyType->isIdentical(*target.mapKeyType) && mapValueType->isIdentical(*target.mapValueType);

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
                if (srcField.name != dstField.name || !srcField.type->isIdentical(*dstField.type)) {
                    return false;
                }
            }

        case Kind::kOptional:
            return optionalValueType->isIdentical(*target.optionalValueType);

        default:
            throw std::runtime_error("not impl");
    }
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

ExpressionType BinaryExpressionNode::getExpressionType() const {
    return ExpressionType::kBinary;
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

ExpressionType CallExpressionNode::getExpressionType() const {
    return ExpressionType::kCall;
}

ConvertExpressionNode::ConvertExpressionNode(
    std::unique_ptr<ExpressionNode> value,
    boost::local_shared_ptr<TypeNode> type,
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

ExpressionType ConvertExpressionNode::getExpressionType() const {
    return ExpressionType::kConvert;
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

ExpressionType DottedExpressionNode::getExpressionType() const {
    return ExpressionType::kDotted;
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

ConstValueExpressionType LiteralArrayExpressionNode::getConstValueExpressionType() const {
    return ConstValueExpressionType::kArray;
}

LiteralBooleanExpressionNode::LiteralBooleanExpressionNode(bool value, Token token)
    : ConstValueExpressionNode(std::move(token)), value(value) {}

void LiteralBooleanExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(LiteralBooleanExpressionNode);
    DUMP_VAR(value);
}

ConstValueExpressionType LiteralBooleanExpressionNode::getConstValueExpressionType() const {
    return ConstValueExpressionType::kBoolean;
}

LiteralNumberExpressionNode::LiteralNumberExpressionNode(decimal::Decimal value, Token token)
    : ConstValueExpressionNode(std::move(token)), value(std::move(value)) {}

void LiteralNumberExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(LiteralNumberExpressionNode);
    DUMP_VAR_DECIMAL(value);
}

ConstValueExpressionType LiteralNumberExpressionNode::getConstValueExpressionType() const {
    return ConstValueExpressionType::kNumber;
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

ConstValueExpressionType LiteralRecordExpressionNode::getConstValueExpressionType() const {
    return ConstValueExpressionType::kRecord;
}

LiteralStringExpressionNode::LiteralStringExpressionNode(std::string value, Token token)
    : ConstValueExpressionNode(std::move(token)), value(std::move(value)) {}

void LiteralStringExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(LiteralStringExpressionNode);
    DUMP_VAR(value);
}

ConstValueExpressionType LiteralStringExpressionNode::getConstValueExpressionType() const {
    return ConstValueExpressionType::kString;
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

ExpressionType NotExpressionNode::getExpressionType() const {
    return ExpressionType::kNot;
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

ExpressionType SymbolReferenceExpressionNode::getExpressionType() const {
    return ExpressionType::kSymbolReference;
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
    : StatementNode(std::move(token)), name(std::move(name)), suffixes(std::move(suffixes)), value(std::move(value)) {}

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

StatementType AssignStatementNode::getStatementType() const {
    return StatementType::kAssign;
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
    : StatementNode(std::move(token)), name(std::move(name)), arguments(std::move(arguments)) {}

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

StatementType CallStatementNode::getStatementType() const {
    return StatementType::kCall;
}

ConstStatementNode::ConstStatementNode(std::string name, std::unique_ptr<ConstValueExpressionNode> value, Token token)
    : StatementNode(std::move(token)), name(std::move(name)), value(std::move(value)) {}

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

StatementType ConstStatementNode::getStatementType() const {
    return StatementType::kConst;
}

ContinueStatementNode::ContinueStatementNode(ContinueScope scope, Token token)
    : StatementNode(std::move(token)), scope(scope) {}

void ContinueStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ContinueStatementNode);
    DUMP_VAR_ENUM(scope);
}

StatementType ContinueStatementNode::getStatementType() const {
    return StatementType::kContinue;
}

DimListStatementNode::DimListStatementNode(std::string name, std::unique_ptr<BodyNode> body, Token token)
    : StatementNode(std::move(token)), name(std::move(name)), body(std::move(body)) {}

void DimListStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DimListStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(body);
}

bool DimListStatementNode::visitBodies(const VisitBodyFunc& func) const {
    return func(*body);
}

StatementType DimListStatementNode::getStatementType() const {
    return StatementType::kDimList;
}

DimMapStatementNode::DimMapStatementNode(std::string name, std::unique_ptr<BodyNode> body, Token token)
    : StatementNode(std::move(token)), name(std::move(name)), body(std::move(body)) {}

void DimMapStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DimMapStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(body);
}

bool DimMapStatementNode::visitBodies(const VisitBodyFunc& func) const {
    return func(*body);
}

StatementType DimMapStatementNode::getStatementType() const {
    return StatementType::kDimMap;
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

StatementType DimStatementNode::getStatementType() const {
    return StatementType::kDim;
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
    : StatementNode(std::move(token)),
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

StatementType DoStatementNode::getStatementType() const {
    return StatementType::kDo;
}

ExitStatementNode::ExitStatementNode(ExitScope scope, Token token) : StatementNode(std::move(token)), scope(scope) {}

void ExitStatementNode::dump(std::ostringstream& s, int n) const {
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

StatementType ForEachStatementNode::getStatementType() const {
    return StatementType::kForEach;
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
    : StatementNode(std::move(token)),
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

StatementType ForStatementNode::getStatementType() const {
    return StatementType::kFor;
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
    : StatementNode(std::move(token)),
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

StatementType GroupStatementNode::getStatementType() const {
    return StatementType::kGroup;
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
    : StatementNode(std::move(token)),
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

StatementType IfStatementNode::getStatementType() const {
    return StatementType::kIf;
}

JoinStatementNode::JoinStatementNode(
    std::string needleName,
    std::unique_ptr<ExpressionNode> haystack,
    std::unique_ptr<ExpressionNode> joinExpression,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(std::move(token)),
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

StatementType JoinStatementNode::getStatementType() const {
    return StatementType::kJoin;
}

RethrowStatementNode::RethrowStatementNode(Token token) : StatementNode(std::move(token)) {}

void RethrowStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(RethrowStatementNode);
}

StatementType RethrowStatementNode::getStatementType() const {
    return StatementType::kRethrow;
}

ReturnStatementNode::ReturnStatementNode(std::unique_ptr<ExpressionNode> expression, Token token)
    : StatementNode(std::move(token)), expression(std::move(expression)) {}

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

StatementType ReturnStatementNode::getStatementType() const {
    return StatementType::kReturn;
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
    : StatementNode(std::move(token)), expression(std::move(expression)), cases(std::move(cases)) {}

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

StatementType SelectCaseStatementNode::getStatementType() const {
    return StatementType::kSelectCase;
}

SelectStatementNode::SelectStatementNode(
    std::unique_ptr<ExpressionNode> expression,
    std::unique_ptr<ExpressionNode> toExpression,
    Token token)
    : StatementNode(std::move(token)), expression(std::move(expression)), toExpression(std::move(toExpression)) {}

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

StatementType SelectStatementNode::getStatementType() const {
    return StatementType::kSelect;
}

ThrowStatementNode::ThrowStatementNode(std::unique_ptr<ExpressionNode> expression, Token token)
    : StatementNode(std::move(token)), expression(std::move(expression)) {}

void ThrowStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ThrowStatementNode);
    DUMP_VAR_NODE(expression);
}

bool ThrowStatementNode::visitExpressions(const VisitExpressionFunc& func) const {
    return func(*expression);
}

StatementType ThrowStatementNode::getStatementType() const {
    return StatementType::kThrow;
}

TryStatementNode::TryStatementNode(
    std::unique_ptr<BodyNode> tryBody,
    std::unique_ptr<BodyNode> catchBody,
    std::unique_ptr<BodyNode> finallyBody,
    Token token)
    : StatementNode(std::move(token)),
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

StatementType TryStatementNode::getStatementType() const {
    return StatementType::kTry;
}

WhileStatementNode::WhileStatementNode(
    std::unique_ptr<ExpressionNode> condition,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(std::move(token)), condition(std::move(condition)), body(std::move(body)) {}

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

StatementType WhileStatementNode::getStatementType() const {
    return StatementType::kWhile;
}

PrintStatementNode::PrintStatementNode(
    std::vector<std::unique_ptr<ExpressionNode>> expressions,
    std::optional<std::string> toIdentifier,
    bool trailingSemicolon,
    Token token)
    : StatementNode(std::move(token)),
      expressions(std::move(expressions)),
      toIdentifier(std::move(toIdentifier)),
      trailingSemicolon(trailingSemicolon) {}

void PrintStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(PrintStatementNode);
    DUMP_VAR_NODES(expressions);
    DUMP_VAR_OPTIONAL(toIdentifier);
    DUMP_VAR(trailingSemicolon);
}

bool PrintStatementNode::visitExpressions(const VisitExpressionFunc& func) const {
    for (const auto& x : expressions) {
        if (!x->visitExpressions(func)) {
            return false;
        }
    }
    return true;
}

StatementType PrintStatementNode::getStatementType() const {
    return StatementType::kPrint;
}

InputStatementNode::InputStatementNode(std::optional<std::string> fromIdentifier, std::string toIdentifier, Token token)
    : StatementNode(std::move(token)),
      fromIdentifier(std::move(fromIdentifier)),
      toIdentifier(std::move(toIdentifier)) {}

void InputStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(InputStatementNode);
    DUMP_VAR_OPTIONAL(fromIdentifier);
    DUMP_VAR(toIdentifier);
}

StatementType InputStatementNode::getStatementType() const {
    return StatementType::kInput;
}

ParameterNode::ParameterNode(std::string name, boost::local_shared_ptr<TypeNode> type, Token token)
    : Node(std::move(token)), name(std::move(name)), type(std::move(type)) {}

void ParameterNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ParameterNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(type);
}

std::optional<std::string> ParameterNode::getSymbolDeclaration() const {
    return name;
}

GlobalVariableNode::GlobalVariableNode(const CompiledGlobalVariable& compiledGlobalVariable)
    : Node(Token(0, 0, TokenKind::kError, "", nullptr)), compiledGlobalVariable(compiledGlobalVariable) {}

void GlobalVariableNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(GlobalVariableNode);
}

std::optional<std::string> GlobalVariableNode::getSymbolDeclaration() const {
    return compiledGlobalVariable.lowercaseName;
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
