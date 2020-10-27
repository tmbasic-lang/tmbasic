#include "Ast.h"
#include "core/util/cast.h"
#include "core/util/decimal.h"

using namespace basic;
using namespace util;

static std::string indent(int n) {
    return std::string(n, ' ');
}

static std::string stripNamespace(std::string_view sv) {
    auto s = std::string(sv);
    return std::regex_replace(s, std::regex("^basic::"), "");
}

#define DUMP_VAR_NAME(f) \
    { s << indent(n + 1) << NAMEOF(f) << '\n'; }

#define DUMP_NODES(f)         \
    for (const auto& x : f) { \
        x->dump(s, n + 2);    \
    }

#define DUMP_TYPE(t) \
    { s << indent(n) << stripNamespace(NAMEOF_TYPE(t)) << '\n'; }

#define DUMP_VAR(f) \
    { s << indent(n + 1) << NAMEOF(f) << '=' << f << '\n'; }

#define DUMP_VAR_OPTIONAL(f)                                  \
    if (f.has_value()) {                                      \
        s << indent(n + 1) << NAMEOF(f) << '=' << *f << '\n'; \
    }

#define DUMP_VAR_DECIMAL(f) \
    { s << indent(n + 1) << NAMEOF(f) << '=' << decimalToString(f) << '\n'; }

#define DUMP_VAR_OPTIONAL_DECIMAL(f)                                           \
    if (f.has_value()) {                                                       \
        s << indent(n + 1) << NAMEOF(f) << '=' << decimalToString(*f) << '\n'; \
    }

#define DUMP_VAR_ENUM(f) \
    { s << indent(n + 1) << NAMEOF(f) << '=' << NAMEOF_ENUM(f) << '\n'; }

#define DUMP_VAR_NODE(f)   \
    if (f) {               \
        DUMP_VAR_NAME(f);  \
        f->dump(s, n + 2); \
    }

#define DUMP_VAR_NODES(f) \
    if (f.size() > 0) {   \
        DUMP_VAR_NAME(f); \
        DUMP_NODES(f);    \
    }

Node::~Node() {}

void Node::dump(std::ostringstream& s, int n) const {
    assert(false);
    DUMP_TYPE(Node);
}

ExpressionNode::ExpressionNode(Token token) : Node(token) {}

ConstValueExpressionNode::ConstValueExpressionNode(Token token) : ExpressionNode(token) {}

StatementNode::StatementNode(Token token) : Node(token) {}

FieldNode::FieldNode(std::string name, std::unique_ptr<TypeNode> type, Token token)
    : Node(token), name(std::move(name)), type(std::move(type)) {}

void FieldNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(FieldNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(type);
}

TypeNode::TypeNode(Kind kind, Token token) : Node(token), kind(kind) {}

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
    : Node(token), binaryOperator(binaryOperator), rightOperand(std::move(rightOperand)) {}

void BinaryExpressionSuffixNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(BinaryExpressionSuffixNode);
    DUMP_VAR_ENUM(binaryOperator);
    DUMP_VAR_NODE(rightOperand);
}

BinaryExpressionNode::BinaryExpressionNode(
    std::unique_ptr<ExpressionNode> leftOperand,
    std::vector<std::unique_ptr<BinaryExpressionSuffixNode>> binarySuffixes,
    Token token)
    : ExpressionNode(token), leftOperand(std::move(leftOperand)), binarySuffixes(std::move(binarySuffixes)) {}

void BinaryExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(BinaryExpressionNode);
    DUMP_VAR_NODE(leftOperand);
    DUMP_VAR_NODES(binarySuffixes);
}

CallExpressionNode::CallExpressionNode(
    std::string name,
    std::vector<std::unique_ptr<ExpressionNode>> arguments,
    Token token)
    : ExpressionNode(token), name(std::move(name)), arguments(std::move(arguments)) {}

void CallExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(CallExpressionNode);
    DUMP_VAR(name);
    DUMP_VAR_NODES(arguments);
}

ConvertExpressionNode::ConvertExpressionNode(
    std::unique_ptr<ExpressionNode> value,
    std::unique_ptr<TypeNode> type,
    Token token)
    : ExpressionNode(token), value(std::move(value)), type(std::move(type)) {}

void ConvertExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ConvertExpressionNode);
    DUMP_VAR_NODE(value);
    DUMP_VAR_NODE(type);
}

DottedExpressionSuffixNode::DottedExpressionSuffixNode(
    std::string name,
    bool isCall,
    std::vector<std::unique_ptr<ExpressionNode>> callArguments,
    Token token)
    : Node(token), name(std::move(name)), isCall(isCall), callArguments(std::move(callArguments)) {}

void DottedExpressionSuffixNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DottedExpressionSuffixNode);
    DUMP_VAR(name);
    DUMP_VAR(isCall);
    DUMP_VAR_NODES(callArguments);
}

DottedExpressionNode::DottedExpressionNode(
    std::unique_ptr<ExpressionNode> base,
    std::vector<std::unique_ptr<DottedExpressionSuffixNode>> dottedSuffixes,
    Token token)
    : ExpressionNode(token), base(std::move(base)), dottedSuffixes(std::move(dottedSuffixes)) {}

void DottedExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DottedExpressionNode);
    DUMP_VAR_NODE(base);
    DUMP_VAR_NODES(dottedSuffixes);
}

LiteralArrayExpressionNode::LiteralArrayExpressionNode(
    std::vector<std::unique_ptr<ExpressionNode>> elements,
    Token token)
    : ConstValueExpressionNode(token), elements(std::move(elements)) {}

void LiteralArrayExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(LiteralArrayExpressionNode);
    DUMP_VAR_NODES(elements);
}

LiteralBooleanExpressionNode::LiteralBooleanExpressionNode(bool value, Token token)
    : ConstValueExpressionNode(token), value(value) {}

void LiteralBooleanExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(LiteralBooleanExpressionNode);
    DUMP_VAR(value);
}

LiteralNumberExpressionNode::LiteralNumberExpressionNode(decimal::Decimal value, Token token)
    : ConstValueExpressionNode(token), value(value) {}

void LiteralNumberExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(LiteralNumberExpressionNode);
    DUMP_VAR_DECIMAL(value);
}

LiteralRecordFieldNode::LiteralRecordFieldNode(std::string key, std::unique_ptr<ExpressionNode> value, Token token)
    : Node(token), key(std::move(key)), value(std::move(value)) {}

void LiteralRecordFieldNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(LiteralRecordFieldNode);
    DUMP_VAR(key);
    DUMP_VAR_NODE(value);
}

LiteralRecordExpressionNode::LiteralRecordExpressionNode(
    std::vector<std::unique_ptr<LiteralRecordFieldNode>> fields,
    Token token)
    : ConstValueExpressionNode(token), fields(std::move(fields)) {}

void LiteralRecordExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(LiteralRecordExpressionNode);
    DUMP_VAR_NODES(fields);
}

LiteralStringExpressionNode::LiteralStringExpressionNode(std::string value, Token token)
    : ConstValueExpressionNode(token), value(std::move(value)) {}

void LiteralStringExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(LiteralStringExpressionNode);
    DUMP_VAR(value);
}

NotExpressionNode::NotExpressionNode(std::unique_ptr<ExpressionNode> operand, Token token)
    : ExpressionNode(token), operand(std::move(operand)) {}

void NotExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(NotExpressionNode);
    DUMP_VAR_NODE(operand);
}

ParenthesesExpressionNode::ParenthesesExpressionNode(std::unique_ptr<ExpressionNode> expression, Token token)
    : ExpressionNode(token), expression(std::move(expression)) {}

void ParenthesesExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ParenthesesExpressionNode);
    DUMP_VAR_NODE(expression);
}

SymbolReferenceExpressionNode::SymbolReferenceExpressionNode(std::string name, Token token)
    : ExpressionNode(token), name(std::move(name)) {}

void SymbolReferenceExpressionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(SymbolReferenceExpressionNode);
    DUMP_VAR(name);
}

AssignLocationSuffixNode::AssignLocationSuffixNode(std::string name, Token token)
    : Node(token), arrayIndex(std::unique_ptr<ExpressionNode>()), name(std::optional<std::string>(name)) {}

AssignLocationSuffixNode::AssignLocationSuffixNode(std::unique_ptr<ExpressionNode> arrayIndex, Token token)
    : Node(token), arrayIndex(std::move(arrayIndex)), name({}) {}

void AssignLocationSuffixNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(AssignLocationSuffixNode);
    DUMP_VAR_OPTIONAL(name);
    DUMP_VAR_NODE(arrayIndex);
}

AssignStatementNode::AssignStatementNode(
    std::string name,
    std::vector<std::unique_ptr<AssignLocationSuffixNode>> suffixes,
    std::unique_ptr<ExpressionNode> value,
    Token token)
    : StatementNode(token), name(std::move(name)), suffixes(std::move(suffixes)), value(std::move(value)) {}

void AssignStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(AssignStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODES(suffixes);
    DUMP_VAR_NODE(value);
}

BodyNode::BodyNode(std::vector<std::unique_ptr<StatementNode>>& statements, Token token)
    : Node(token), statements(std::move(statements)) {}

void BodyNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(BodyNode);
    DUMP_VAR_NODES(statements);
}

CallStatementNode::CallStatementNode(
    std::string name,
    std::vector<std::unique_ptr<ExpressionNode>> arguments,
    Token token)
    : StatementNode(token), name(std::move(name)), arguments(std::move(arguments)) {}

void CallStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(CallStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODES(arguments);
}

ConstStatementNode::ConstStatementNode(std::string name, std::unique_ptr<ConstValueExpressionNode> value, Token token)
    : StatementNode(token), name(std::move(name)), value(std::move(value)) {}

MemberType ConstStatementNode::getMemberType() const {
    return MemberType::kConstStatement;
}

void ConstStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ConstStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(value);
}

ContinueStatementNode::ContinueStatementNode(ContinueScope scope, Token token) : StatementNode(token), scope(scope) {}

void ContinueStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ContinueStatementNode);
    DUMP_VAR_ENUM(scope);
}

void DimListStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DimListStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(body);
}

void DimMapStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DimMapStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(body);
}

DimStatementNode::DimStatementNode(std::string name, std::unique_ptr<TypeNode> type, Token token)
    : StatementNode(token), name(std::move(name)), type(std::move(type)), value(std::unique_ptr<ExpressionNode>()) {}

DimStatementNode::DimStatementNode(std::string name, std::unique_ptr<ExpressionNode> value, Token token)
    : StatementNode(token), name(std::move(name)), type(std::unique_ptr<TypeNode>()), value(std::move(value)) {}

MemberType DimStatementNode::getMemberType() const {
    return MemberType::kDimStatement;
}

void DimStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DimStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(type);
    DUMP_VAR_NODE(value);
}

DimCollectionStatementNode::DimCollectionStatementNode(
    std::string name,
    CollectionType type,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(token), name(std::move(name)), type(type), body(std::move(body)) {}

void DimCollectionStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DimCollectionStatementNode);
    DUMP_VAR(name);
    DUMP_VAR_ENUM(type);
    DUMP_VAR_NODE(body);
}

DoConditionNode::DoConditionNode(std::unique_ptr<ExpressionNode> condition, DoConditionType conditionType, Token token)
    : Node(token), condition(std::move(condition)), conditionType(conditionType) {}

void DoConditionNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DoConditionNode);
    DUMP_VAR_NODE(condition);
    DUMP_VAR_ENUM(conditionType);
}

DoStatementNode::DoStatementNode(
    std::unique_ptr<DoConditionNode> condition,
    DoConditionPosition conditionPosition,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(token),
      condition(std::move(condition)),
      conditionPosition(conditionPosition),
      body(std::move(body)) {}

void DoStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(DoStatementNode);
    DUMP_VAR_NODE(condition);
    DUMP_VAR_ENUM(conditionPosition);
    DUMP_VAR_NODE(body);
}

ExitStatementNode::ExitStatementNode(ExitScope scope, Token token) : StatementNode(token), scope(scope) {}

void ExitStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ExitStatementNode);
    DUMP_VAR_ENUM(scope);
}

ForEachStatementNode::ForEachStatementNode(
    std::string needleName,
    std::unique_ptr<ExpressionNode> haystack,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(token), needleName(std::move(needleName)), haystack(std::move(haystack)), body(std::move(body)) {}

void ForEachStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ForEachStatementNode);
    DUMP_VAR(needleName);
    DUMP_VAR_NODE(haystack);
    DUMP_VAR_NODE(body);
}

ForStepNode::ForStepNode(decimal::Decimal stepImmediate, Token token) : Node(token), stepImmediate(stepImmediate) {}

ForStepNode::ForStepNode(std::unique_ptr<SymbolReferenceExpressionNode> stepConstant, Token token)
    : Node(token), stepConstant(std::move(stepConstant)) {}

void ForStepNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ForStepNode);
    DUMP_VAR_OPTIONAL_DECIMAL(stepImmediate);
    DUMP_VAR_NODE(stepConstant);
}

ForStatementNode::ForStatementNode(
    std::string loopVariableName,
    std::unique_ptr<ExpressionNode> fromValue,
    std::unique_ptr<ExpressionNode> toValue,
    std::unique_ptr<ForStepNode> step,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(token),
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

GroupKeyNameNode::GroupKeyNameNode(std::string name, Token token) : Node(token), name(std::move(name)) {}

void GroupKeyNameNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(GroupKeyNameNode);
    DUMP_VAR(name);
}

GroupStatementNode::GroupStatementNode(
    std::unique_ptr<ExpressionNode> itemExpression,
    std::unique_ptr<ExpressionNode> groupingExpression,
    std::string groupName,
    std::unique_ptr<GroupKeyNameNode> keyName,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(token),
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

ElseIfNode::ElseIfNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<BodyNode> body, Token token)
    : Node(token), condition(std::move(condition)), body(std::move(body)) {}

void ElseIfNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ElseIfNode);
    DUMP_VAR_NODE(condition);
    DUMP_VAR_NODE(body);
}

IfStatementNode::IfStatementNode(
    std::unique_ptr<ExpressionNode> condition,
    std::unique_ptr<BodyNode> body,
    std::vector<std::unique_ptr<ElseIfNode>> elseIfs,
    std::unique_ptr<BodyNode> elseBody,
    Token token)
    : StatementNode(token),
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

JoinStatementNode::JoinStatementNode(
    std::string needleName,
    std::unique_ptr<ExpressionNode> haystack,
    std::unique_ptr<ExpressionNode> joinExpression,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(token),
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

RethrowStatementNode::RethrowStatementNode(Token token) : StatementNode(token) {}

void RethrowStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(RethrowStatementNode);
}

ReturnStatementNode::ReturnStatementNode(std::unique_ptr<ExpressionNode> expression, Token token)
    : StatementNode(token), expression(std::move(expression)) {}

void ReturnStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ReturnStatementNode);
    DUMP_VAR_NODE(expression);
}

CaseValueNode::CaseValueNode(
    std::unique_ptr<ExpressionNode> expression,
    std::unique_ptr<ExpressionNode> toExpression,
    Token token)
    : Node(token), expression(std::move(expression)), toExpression(std::move(toExpression)) {}

void CaseValueNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(CaseValueNode);
    DUMP_VAR_NODE(expression);
    DUMP_VAR_NODE(toExpression);
}

CaseNode::CaseNode(std::vector<std::unique_ptr<CaseValueNode>> values, std::unique_ptr<BodyNode> body, Token token)
    : Node(token), values(std::move(values)), body(std::move(body)) {}

void CaseNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(CaseNode);
    DUMP_VAR_NODES(values);
    DUMP_VAR_NODE(body);
}

SelectCaseStatementNode::SelectCaseStatementNode(
    std::unique_ptr<ExpressionNode> expression,
    std::vector<std::unique_ptr<CaseNode>> cases,
    Token token)
    : StatementNode(token), expression(std::move(expression)), cases(std::move(cases)) {}

void SelectCaseStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(SelectCaseStatementNode);
    DUMP_VAR_NODE(expression);
    DUMP_VAR_NODES(cases);
}

SelectStatementNode::SelectStatementNode(
    std::unique_ptr<ExpressionNode> expression,
    std::unique_ptr<ExpressionNode> toExpression,
    Token token)
    : StatementNode(token), expression(std::move(expression)), toExpression(std::move(toExpression)) {}

void SelectStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(SelectStatementNode);
    DUMP_VAR_NODE(expression);
    DUMP_VAR_NODE(toExpression);
}

ThrowStatementNode::ThrowStatementNode(std::unique_ptr<ExpressionNode> expression, Token token)
    : StatementNode(token), expression(std::move(expression)) {}

void ThrowStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ThrowStatementNode);
    DUMP_VAR_NODE(expression);
}

TryStatementNode::TryStatementNode(
    std::unique_ptr<BodyNode> tryBody,
    std::unique_ptr<BodyNode> catchBody,
    std::unique_ptr<BodyNode> finallyBody,
    Token token)
    : StatementNode(token),
      tryBody(std::move(tryBody)),
      catchBody(std::move(catchBody)),
      finallyBody(std::move(finallyBody)) {}

void TryStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(TryStatementNode);
    DUMP_VAR_NODE(tryBody);
    DUMP_VAR_NODE(catchBody);
    DUMP_VAR_NODE(finallyBody);
}

WhileStatementNode::WhileStatementNode(
    std::unique_ptr<ExpressionNode> condition,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(token), condition(std::move(condition)), body(std::move(body)) {}

void WhileStatementNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(WhileStatementNode);
    DUMP_VAR_NODE(condition);
    DUMP_VAR_NODE(body);
}

ParameterNode::ParameterNode(std::string name, std::unique_ptr<TypeNode> type, Token token)
    : Node(token), name(std::move(name)), type(std::move(type)) {}

void ParameterNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ParameterNode);
    DUMP_VAR(name);
    DUMP_VAR_NODE(type);
}

ProcedureNode::ProcedureNode(
    std::string name,
    std::vector<std::unique_ptr<ParameterNode>> parameters,
    std::unique_ptr<TypeNode> returnType,
    std::unique_ptr<BodyNode> body,
    Token token)
    : Node(token),
      name(std::move(name)),
      parameters(std::move(parameters)),
      returnType(std::move(returnType)),
      body(std::move(body)) {}

ProcedureNode::ProcedureNode(
    std::string name,
    std::vector<std::unique_ptr<ParameterNode>> parameters,
    std::unique_ptr<BodyNode> body,
    Token token)
    : Node(token), name(std::move(name)), parameters(std::move(parameters)), body(std::move(body)) {}

void ProcedureNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ProcedureNode);
    DUMP_VAR(name);
    DUMP_VAR_NODES(parameters);
    DUMP_VAR_NODE(returnType);
    DUMP_VAR_NODE(body);
}

TypeDeclarationNode::TypeDeclarationNode(
    std::string name,
    std::vector<std::unique_ptr<ParameterNode>> fields,
    Token token)
    : Node(token), name(std::move(name)), fields(std::move(fields)) {}

MemberType TypeDeclarationNode::getMemberType() const {
    return MemberType::kTypeDeclaration;
}

void TypeDeclarationNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(TypeDeclarationNode);
    DUMP_VAR(name);
    DUMP_VAR_NODES(fields);
}

ProgramNode::ProgramNode(std::vector<std::unique_ptr<Node>> members, Token token)
    : Node(token), members(std::move(members)) {}

MemberType ProcedureNode::getMemberType() const {
    return MemberType::kProcedure;
}

void ProgramNode::dump(std::ostringstream& s, int n) const {
    DUMP_TYPE(ProgramNode);
    DUMP_VAR_NODES(members);
}
