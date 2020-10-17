#include "Ast.h"

using namespace basic;

Node::~Node() {}

FieldNode::FieldNode(std::string name, std::unique_ptr<TypeNode> type, Token token)
    : Node(token), name(std::move(name)), type(std::move(type)) {}

TypeNode::TypeNode(Kind kind, Token token) : Node(token), kind(kind) {}

BodyNode::BodyNode(std::vector<std::unique_ptr<StatementNode>>& statements, Token token)
    : Node(token), statements(std::move(statements)) {}

ParameterNode::ParameterNode(std::string name, std::unique_ptr<TypeNode> type, Token token)
    : Node(token), name(std::move(name)), type(std::move(type)) {}

ExpressionNode::ExpressionNode(Token token) : Node(token) {}

ConstValueExpressionNode::ConstValueExpressionNode(Token token) : ExpressionNode(token) {}

LiteralBooleanExpressionNode::LiteralBooleanExpressionNode(bool value, Token token)
    : ConstValueExpressionNode(token), value(value) {}

LiteralNumberExpressionNode::LiteralNumberExpressionNode(std::decimal::decimal64 value, Token token)
    : ConstValueExpressionNode(token), value(value) {}

LiteralStringExpressionNode::LiteralStringExpressionNode(std::string value, Token token)
    : ConstValueExpressionNode(token), value(std::move(value)) {}

LiteralRecordFieldNode::LiteralRecordFieldNode(std::string key, std::unique_ptr<ExpressionNode> value, Token token)
    : Node(token), key(std::move(key)), value(std::move(value)) {}

LiteralRecordExpressionNode::LiteralRecordExpressionNode(
    std::vector<std::unique_ptr<LiteralRecordFieldNode>>,
    Token token)
    : ConstValueExpressionNode(token), fields(std::move(fields)) {}

LiteralArrayExpressionNode::LiteralArrayExpressionNode(
    std::vector<std::unique_ptr<ExpressionNode>> elements,
    Token token)
    : ConstValueExpressionNode(token), elements(std::move(elements)) {}

CallExpressionNode::CallExpressionNode(
    std::string name,
    std::vector<std::unique_ptr<ExpressionNode>> arguments,
    Token token)
    : ExpressionNode(token), name(std::move(name)), arguments(std::move(arguments)) {}

ParenthesesExpressionNode::ParenthesesExpressionNode(std::unique_ptr<ExpressionNode> expression, Token token)
    : ExpressionNode(token), expression(std::move(expression)) {}

SymbolReferenceExpressionNode::SymbolReferenceExpressionNode(std::string name, Token token)
    : ExpressionNode(token), name(std::move(name)) {}

DottedExpressionSuffixNode::DottedExpressionSuffixNode(
    std::string name,
    bool isCall,
    std::vector<std::unique_ptr<ExpressionNode>> callArguments,
    Token token)
    : Node(token), name(std::move(name)), isCall(isCall), callArguments(std::move(callArguments)) {}

DottedExpressionNode::DottedExpressionNode(
    std::unique_ptr<ExpressionNode> base,
    std::vector<std::unique_ptr<DottedExpressionSuffixNode>> dottedSuffixes,
    Token token)
    : ExpressionNode(token), base(std::move(base)), dottedSuffixes(std::move(dottedSuffixes)) {}

ConvertExpressionNode::ConvertExpressionNode(
    std::unique_ptr<ExpressionNode> value,
    std::unique_ptr<TypeNode> type,
    Token token)
    : ExpressionNode(token), value(std::move(value)), type(std::move(type)) {}

NotExpressionNode::NotExpressionNode(std::unique_ptr<ExpressionNode> operand, Token token)
    : ExpressionNode(token), operand(std::move(operand)) {}

BinaryExpressionSuffixNode::BinaryExpressionSuffixNode(
    BinaryOperator binaryOperator,
    std::unique_ptr<ExpressionNode> rightOperand,
    Token token)
    : Node(token), binaryOperator(binaryOperator), rightOperand(std::move(rightOperand)) {}

BinaryExpressionNode::BinaryExpressionNode(
    std::unique_ptr<ExpressionNode> leftOperand,
    std::vector<std::unique_ptr<BinaryExpressionSuffixNode>> binarySuffixes,
    Token token)
    : ExpressionNode(token), leftOperand(std::move(leftOperand)), binarySuffixes(std::move(binarySuffixes)) {}

StatementNode::StatementNode(Token token) : Node(token) {}

TryStatementNode::TryStatementNode(
    std::unique_ptr<BodyNode> tryBody,
    std::unique_ptr<BodyNode> catchBody,
    std::unique_ptr<BodyNode> finallyBody,
    Token token)
    : StatementNode(token),
      tryBody(std::move(tryBody)),
      catchBody(std::move(catchBody)),
      finallyBody(std::move(finallyBody)) {}

RethrowStatementNode::RethrowStatementNode(Token token) : StatementNode(token) {}

ThrowStatementNode::ThrowStatementNode(std::unique_ptr<ExpressionNode> expression, Token token)
    : StatementNode(token), expression(std::move(expression)) {}

ExitStatementNode::ExitStatementNode(ExitScope scope, Token token) : StatementNode(token), scope(scope) {}

ContinueStatementNode::ContinueStatementNode(ContinueScope scope, Token token) : StatementNode(token), scope(scope) {}

CallStatementNode::CallStatementNode(
    std::string name,
    std::vector<std::unique_ptr<ExpressionNode>> arguments,
    Token token)
    : StatementNode(token), name(std::move(name)), arguments(std::move(arguments)) {}

ReturnStatementNode::ReturnStatementNode(std::unique_ptr<ExpressionNode> expression, Token token)
    : StatementNode(token), expression(std::move(expression)) {}

SelectStatementNode::SelectStatementNode(
    std::unique_ptr<ExpressionNode> expression,
    std::unique_ptr<ExpressionNode> toExpression,
    Token token)
    : StatementNode(token), expression(std::move(expression)), toExpression(std::move(toExpression)) {}

ConstStatementNode::ConstStatementNode(std::string name, std::unique_ptr<ConstValueExpressionNode> value, Token token)
    : StatementNode(token), name(std::move(name)), value(std::move(value)) {}

MemberType ConstStatementNode::getMemberType() {
    return MemberType::kConstStatement;
}

AssignLocationSuffixNode::AssignLocationSuffixNode(std::string name, Token token)
    : Node(token), arrayIndex(std::unique_ptr<ExpressionNode>()), name(std::optional(name)) {}

AssignLocationSuffixNode::AssignLocationSuffixNode(std::unique_ptr<ExpressionNode> arrayIndex, Token token)
    : Node(token), arrayIndex(std::move(arrayIndex)), name({}) {}

AssignStatementNode::AssignStatementNode(
    std::string name,
    std::vector<std::unique_ptr<AssignLocationSuffixNode>> suffixes,
    std::unique_ptr<ExpressionNode> value,
    Token token)
    : StatementNode(token), name(std::move(name)), suffixes(std::move(suffixes)), value(std::move(value)) {}

DimStatementNode::DimStatementNode(std::string name, std::unique_ptr<TypeNode> type, Token token)
    : StatementNode(token), name(std::move(name)), type(std::move(type)), value(std::unique_ptr<ExpressionNode>()) {}

DimStatementNode::DimStatementNode(std::string name, std::unique_ptr<ExpressionNode> value, Token token)
    : StatementNode(token), name(std::move(name)), type(std::unique_ptr<TypeNode>()), value(std::move(value)) {}

MemberType DimStatementNode::getMemberType() {
    return MemberType::kDimStatement;
}

DimCollectionStatementNode::DimCollectionStatementNode(
    std::string name,
    CollectionType type,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(token), name(std::move(name)), type(type), body(std::move(body)) {}

CaseValueNode::CaseValueNode(
    std::unique_ptr<ExpressionNode> expression,
    std::unique_ptr<ExpressionNode> toExpression,
    Token token)
    : Node(token), expression(std::move(expression)), toExpression(std::move(toExpression)) {}

CaseNode::CaseNode(std::vector<std::unique_ptr<CaseValueNode>>, std::unique_ptr<BodyNode> body, Token token)
    : Node(token), values(std::move(values)), body(std::move(body)) {}

SelectCaseStatementNode::SelectCaseStatementNode(
    std::unique_ptr<ExpressionNode> expression,
    std::vector<std::unique_ptr<CaseNode>> cases,
    Token token)
    : StatementNode(token), expression(std::move(expression)), cases(std::move(cases)) {}

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

GroupKeyNameNode::GroupKeyNameNode(std::string name, Token token) : Node(token), name(std::move(name)) {}

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

DoConditionNode::DoConditionNode(std::unique_ptr<ExpressionNode> condition, DoConditionType conditionType, Token token)
    : Node(token), condition(std::move(condition)), conditionType(conditionType) {}

DoStatementNode::DoStatementNode(
    std::unique_ptr<DoConditionNode> condition,
    DoConditionPosition conditionPosition,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(token),
      condition(std::move(condition)),
      conditionPosition(conditionPosition),
      body(std::move(body)) {}

ElseIfNode::ElseIfNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<BodyNode> body, Token token)
    : Node(token), condition(std::move(condition)), body(std::move(body)) {}

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

WhileStatementNode::WhileStatementNode(
    std::unique_ptr<ExpressionNode> condition,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(token), condition(std::move(condition)), body(std::move(body)) {}

ForEachStatementNode::ForEachStatementNode(
    std::string needleName,
    std::unique_ptr<ExpressionNode> haystack,
    std::unique_ptr<BodyNode> body,
    Token token)
    : StatementNode(token), needleName(std::move(needleName)), haystack(std::move(haystack)), body(std::move(body)) {}

ForStepNode::ForStepNode(std::decimal::decimal64 stepImmediate, Token token)
    : Node(token), stepImmediate(stepImmediate) {}

ForStepNode::ForStepNode(std::unique_ptr<SymbolReferenceExpressionNode> stepConstant, Token token)
    : Node(token), stepConstant(std::move(stepConstant)) {}

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

TypeDeclarationNode::TypeDeclarationNode(
    std::string name,
    std::vector<std::unique_ptr<ParameterNode>> fields,
    Token token)
    : Node(token), name(std::move(name)), fields(std::move(fields)) {}

MemberType TypeDeclarationNode::getMemberType() {
    return MemberType::kTypeDeclaration;
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

MemberType ProcedureNode::getMemberType() {
    return MemberType::kProcedure;
}

ProgramNode::ProgramNode(std::vector<std::unique_ptr<Member>> members, Token token)
    : Node(token), members(std::move(members)) {}
