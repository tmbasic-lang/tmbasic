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
