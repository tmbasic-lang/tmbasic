#include "Ast.h"

using namespace basic;

Node::~Node() {}

FieldNode::FieldNode(std::string name, std::unique_ptr<TypeNode> type, Token token)
    : Node(token), name(std::move(name)), type(std::move(type)) {}

TypeNode::TypeNode(Kind kind, Token token) : Node(token), kind(kind) {}

BodyNode::BodyNode(std::vector<std::unique_ptr<StatementNode>>& statements, Token token)
    : Node(token), statements(std::move(statements)) {}

ParameterNode::ParameterNode(std::string name, std::unique_ptr<TypeNode> type, Token token)
    : Node(token), name(name), type(std::move(type)) {}

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
