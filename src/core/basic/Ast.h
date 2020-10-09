#pragma once

#include "common.h"
#include "Token.h"

namespace basic {

class Node {
   public:
    Token token;
    Node(Token token) : token(token) {}
    virtual ~Node() = 0;
};

//
// types
//

enum class Kind {
    kBoolean,
    kInteger,
    kDecimal,
    kString,
    kList,
    kMap,
    kRecord,
    kProcedureReference,
    kGenericPlaceholder,
    kOptional
};

class TypeNode;

class FieldNode : Node {
   public:
    std::string name;
    std::unique_ptr<TypeNode> type;
    FieldNode(std::string name, std::unique_ptr<TypeNode> type, Token token);
};

class TypeNode : Node {
   public:
    Kind kind;
    std::optional<std::string> recordName;              // kind = kRecord (named)
    std::vector<std::unique_ptr<FieldNode>> fields;     // kind = kRecord (anonymous)
    std::optional<std::string> genericPlaceholderName;  // kind = kGenericPlaceholder
    // nullable type parameters
    std::unique_ptr<TypeNode> listItemType;       // kind = kList
    std::unique_ptr<TypeNode> mapKeyType;         // kind = kMap
    std::unique_ptr<TypeNode> mapValueType;       // kind = kMap
    std::unique_ptr<TypeNode> optionalValueType;  // kind = kOptional
    TypeNode(Kind kind, Token token);
};

//
// expressions
//

class ExpressionNode : public Node {};

enum class BinaryOperator {
    kOr,
    kAnd,
    kEquals,
    kNotEquals,
    kLessThan,
    kLessThanEquals,
    kGreaterThan,
    kGreaterThanEquals,
    kAdd,
    kSubtract,
    kMultiply,
    kDivide,
    kModulus
};

class BinaryExpressionSuffixNode : public Node {
   public:
    BinaryOperator binaryOperator;
    std::unique_ptr<ExpressionNode> rightOperand;
};

class BinaryExpressionNode : public ExpressionNode {
   public:
    std::unique_ptr<ExpressionNode> leftOperand;
    std::vector<std::unique_ptr<BinaryExpressionSuffixNode>> binarySuffixes;
};

class CallExpressionNode : public ExpressionNode {
   public:
    std::string name;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;
};

class ConstValueExpressionNode : public ExpressionNode {};

class ConvertExpressionNode : public ExpressionNode {
   public:
    std::unique_ptr<ExpressionNode> value;
    std::unique_ptr<TypeNode> type;
};

class DottedExpressionSuffixNode : public Node {
   public:
    std::string name;
    bool isCall;
    std::vector<std::unique_ptr<ExpressionNode>> callArguments;
};

class DottedExpressionNode : public ExpressionNode {
   public:
    std::unique_ptr<ExpressionNode> base;
    std::vector<std::unique_ptr<DottedExpressionSuffixNode>> dottedSuffixes;
};

class LiteralArrayExpressionNode : public ConstValueExpressionNode {
   public:
    std::vector<std::unique_ptr<ExpressionNode>> elements;
};

class LiteralBooleanExpressionNode : public ConstValueExpressionNode {
   public:
    bool value;
};

class LiteralIntegerExpressionNode : public ConstValueExpressionNode {
   public:
    int64_t value;
};

class LiteralRecordFieldNode : public Node {
   public:
    std::string key;
    std::unique_ptr<ExpressionNode> value;
};

class LiteralRecordExpressionNode : public ConstValueExpressionNode {
   public:
    std::vector<std::unique_ptr<LiteralRecordFieldNode>> fields;
};

class LiteralStringExpressionNode : public ConstValueExpressionNode {
   public:
    std::string value;
};

class NotExpressionNode : public ExpressionNode {
   public:
    std::unique_ptr<ExpressionNode> operand;
};

class ParenthesesExpressionNode : public ExpressionNode {
   public:
    std::unique_ptr<ExpressionNode> expression;
};

class SymbolReferenceExpressionNode : public ExpressionNode {
   public:
    std::string name;
};

//
// statements
//

class StatementNode : public Node {};

class AssignStatementNode : public StatementNode {
   public:
    std::string name;
    std::unique_ptr<ExpressionNode> value;
};

class BodyNode : public Node {
   public:
    std::vector<std::unique_ptr<StatementNode>> statements;
    BodyNode(std::vector<std::unique_ptr<StatementNode>>& statements, Token token);
};

class CallStatementNode : public StatementNode {
   public:
    std::string name;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;
};

class ConstStatementNode : public StatementNode {
   public:
    std::string name;
    std::unique_ptr<ConstValueExpressionNode> value;
};

enum class ContinueScope { kDo, kFor, kForEach, kWhile };

class ContinueStatementNode : public StatementNode {
   public:
    ContinueScope scope;
};

class DimListStatementNode : public StatementNode {
   public:
    std::string name;
    std::unique_ptr<BodyNode> body;
};

class DimMapStatementNode : public StatementNode {
   public:
    std::string name;
    std::unique_ptr<BodyNode> body;
};

class DimStatementNode : public StatementNode {
   public:
    std::string name;
    std::unique_ptr<TypeNode> type;
    std::unique_ptr<ExpressionNode> value;  // may be null
};

enum class DoConditionPosition { kBeforeBody, kAfterBody };

enum class DoConditionType { kWhile, kUntil };

class DoStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> condition;
    DoConditionType conditionType;
    DoConditionPosition conditionPosition;
    std::unique_ptr<BodyNode> body;
};

enum class ExitScope { kDo, kFor, kForEach, kSelectCase, kTry, kWhile };

class ExitStatementNode : public StatementNode {
   public:
    ExitScope scope;
};

class ForEachStatementNode : public StatementNode {
   public:
    std::string needleName;
    std::unique_ptr<ExpressionNode> haystack;
    std::unique_ptr<BodyNode> body;
};

class ForStepNode : public Node {
   public:
    std::optional<int64_t> stepImmediate;
    std::unique_ptr<SymbolReferenceExpressionNode> stepConstant;  // may be null
};

class ForStatementNode : public StatementNode {
   public:
    std::string loopVariableName;
    std::unique_ptr<ExpressionNode> fromValue;
    std::unique_ptr<ExpressionNode> toValue;
    std::unique_ptr<ForStepNode> step;  // may be null
    std::unique_ptr<BodyNode> body;
};

class GroupKeyNameNode : public Node {
   public:
    std::string name;
};

class GroupStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> itemExpression;
    std::unique_ptr<ExpressionNode> groupingExpression;
    std::string groupName;
    std::unique_ptr<GroupKeyNameNode> keyName;  // may be null
    std::unique_ptr<BodyNode> body;
};

class ElseIfNode : public Node {
   public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BodyNode> body;
};

class IfStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BodyNode> body;
    std::vector<std::unique_ptr<ElseIfNode>> elseIfs;
    std::unique_ptr<BodyNode> elseBody;  // may be null
};

class JoinStatementNode : public StatementNode {
   public:
    std::string needleName;
    std::unique_ptr<ExpressionNode> haystack;
    std::unique_ptr<ExpressionNode> joinExpression;
    std::unique_ptr<BodyNode> body;
};

class RethrowStatementNode : public StatementNode {};

class ReturnStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> expression;  // may be null
};

class CaseValueNode : public Node {
   public:
    std::unique_ptr<ExpressionNode> expression;
    std::unique_ptr<ExpressionNode> toExpression;  // may be null
};

class CaseNode : public Node {
   public:
    std::unique_ptr<CaseValueNode> values;  // may be null (for default)
    std::unique_ptr<BodyNode> body;
};

class SelectCaseStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> expression;
    std::vector<std::unique_ptr<CaseNode>> cases;
};

class SelectStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> expression;
    std::unique_ptr<ExpressionNode> toExpression;  // may be null
};

class ThrowStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> expression;
};

class TryStatementNode : public StatementNode {
   public:
    std::unique_ptr<BodyNode> tryBody;
    std::unique_ptr<BodyNode> catchBody;    // may be null
    std::unique_ptr<BodyNode> finallyBody;  // may be null
};

class WhileStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BodyNode> body;
};

// members

class ParameterNode : public Node {
   public:
    std::string name;
    std::unique_ptr<TypeNode> type;
    ParameterNode(std::string name, std::unique_ptr<TypeNode> type, Token token);
};

class ProcedureNode : public Node {
   public:
    std::string name;
    std::vector<std::unique_ptr<ParameterNode>> parameters;
    std::unique_ptr<TypeNode> returnType;  // null for subroutines
    std::unique_ptr<BodyNode> body;
};

}  // namespace basic
