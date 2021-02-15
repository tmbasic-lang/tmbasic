#pragma once

#include "../../common.h"
#include "shared/basic/Token.h"

namespace basic {

enum class MemberType {
    kNonMember,
    kProcedure,
    kDimStatement,
    kConstStatement,
    kTypeDeclaration,
};

class BodyNode;
class ExpressionNode;
class TypeNode;

typedef std::function<bool(BodyNode&)> VisitBodyFunc;
typedef std::function<bool(ExpressionNode&)> VisitExpressionFunc;

class Node {
   public:
    Token token;
    explicit Node(Token token);
    virtual ~Node();
    virtual void dump(std::ostringstream& s, int n) const;
    virtual MemberType getMemberType() const;
    virtual bool visitBodies(const VisitBodyFunc& func) const;
    virtual bool visitExpressions(const VisitExpressionFunc& func) const;
    virtual bool isSymbolReference() const;
    virtual TypeNode* getChildTypeNode() const;  // statements and expressions

    // symbol declaration nodes
    virtual std::optional<std::string> getSymbolDeclaration() const;
    virtual Node* getChildSymbolDeclaration() const;  // a sub-node that declares another symbol
    virtual bool isSymbolVisibleToSiblingStatements() const;
};

//
// types
//

enum class Kind {
    kBoolean,
    kNumber,
    kDate,
    kDateTime,
    kDateTimeOffset,
    kTimeSpan,
    kText,
    kList,
    kMap,
    kRecord,
    kProcedureReference,
    kGenericPlaceholder,
    kOptional,
};

class TypeNode;

class FieldNode : public Node {
   public:
    std::string name;
    std::unique_ptr<TypeNode> type;
    FieldNode(std::string name, std::unique_ptr<TypeNode> type, Token token);
    void dump(std::ostringstream& s, int n) const override;
};

class TypeNode : public Node {
   public:
    Kind kind;
    std::optional<std::string> recordName;              // kind = kRecord (named)
    std::optional<std::string> genericPlaceholderName;  // kind = kGenericPlaceholder
    std::vector<std::unique_ptr<FieldNode>> fields;     // kind = kRecord (anonymous)
    // nullable type parameters
    std::unique_ptr<TypeNode> listItemType;       // kind = kList
    std::unique_ptr<TypeNode> mapKeyType;         // kind = kMap
    std::unique_ptr<TypeNode> mapValueType;       // kind = kMap
    std::unique_ptr<TypeNode> optionalValueType;  // kind = kOptional
    TypeNode(Kind kind, Token token);
    TypeNode(Kind kind, Token token, std::string recordName);
    TypeNode(Kind kind, Token token, std::unique_ptr<TypeNode> optionalValueTypeOrListItemType);
    TypeNode(Kind kind, Token token, std::unique_ptr<TypeNode> mapKeyType, std::unique_ptr<TypeNode> mapValueType);
    TypeNode(Kind kind, Token token, std::vector<std::unique_ptr<FieldNode>> fields);
    void dump(std::ostringstream& s, int n) const override;
};

//
// expressions
//

class ExpressionNode : public Node {
   public:
    explicit ExpressionNode(Token token);
};

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
    kModulus,
};

class BinaryExpressionSuffixNode : public Node {
   public:
    BinaryOperator binaryOperator;
    std::unique_ptr<ExpressionNode> rightOperand;
    BinaryExpressionSuffixNode(
        BinaryOperator binaryOperator,
        std::unique_ptr<ExpressionNode> rightOperand,
        Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class BinaryExpressionNode : public ExpressionNode {
   public:
    std::unique_ptr<ExpressionNode> leftOperand;
    std::vector<std::unique_ptr<BinaryExpressionSuffixNode>> binarySuffixes;
    BinaryExpressionNode(
        std::unique_ptr<ExpressionNode> leftOperand,
        std::vector<std::unique_ptr<BinaryExpressionSuffixNode>> binarySuffixes,
        Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class CallExpressionNode : public ExpressionNode {
   public:
    std::string name;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;
    CallExpressionNode(std::string name, std::vector<std::unique_ptr<ExpressionNode>> arguments, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class ConstValueExpressionNode : public ExpressionNode {
   public:
    explicit ConstValueExpressionNode(Token token);
};

class ConvertExpressionNode : public ExpressionNode {
   public:
    std::unique_ptr<ExpressionNode> value;
    std::unique_ptr<TypeNode> type;
    ConvertExpressionNode(std::unique_ptr<ExpressionNode> value, std::unique_ptr<TypeNode> type, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
    TypeNode* getChildTypeNode() const override;
};

class DottedExpressionSuffixNode : public Node {
   public:
    std::string name;
    bool isCall;
    std::vector<std::unique_ptr<ExpressionNode>> callArguments;
    DottedExpressionSuffixNode(
        std::string name,
        bool isCall,
        std::vector<std::unique_ptr<ExpressionNode>> callArguments,
        Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class DottedExpressionNode : public ExpressionNode {
   public:
    std::unique_ptr<ExpressionNode> base;
    std::vector<std::unique_ptr<DottedExpressionSuffixNode>> dottedSuffixes;
    DottedExpressionNode(
        std::unique_ptr<ExpressionNode> base,
        std::vector<std::unique_ptr<DottedExpressionSuffixNode>> dottedSuffixes,
        Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class LiteralArrayExpressionNode : public ConstValueExpressionNode {
   public:
    std::vector<std::unique_ptr<ExpressionNode>> elements;
    LiteralArrayExpressionNode(std::vector<std::unique_ptr<ExpressionNode>> elements, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class LiteralBooleanExpressionNode : public ConstValueExpressionNode {
   public:
    bool value;
    LiteralBooleanExpressionNode(bool value, Token token);
    void dump(std::ostringstream& s, int n) const override;
};

class LiteralNumberExpressionNode : public ConstValueExpressionNode {
   public:
    decimal::Decimal value;
    LiteralNumberExpressionNode(decimal::Decimal value, Token token);
    void dump(std::ostringstream& s, int n) const override;
};

class LiteralRecordFieldNode : public Node {
   public:
    std::string key;
    std::unique_ptr<ExpressionNode> value;
    LiteralRecordFieldNode(std::string key, std::unique_ptr<ExpressionNode> value, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class LiteralRecordExpressionNode : public ConstValueExpressionNode {
   public:
    std::vector<std::unique_ptr<LiteralRecordFieldNode>> fields;
    LiteralRecordExpressionNode(std::vector<std::unique_ptr<LiteralRecordFieldNode>>, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class LiteralStringExpressionNode : public ConstValueExpressionNode {
   public:
    std::string value;
    LiteralStringExpressionNode(std::string value, Token token);
    void dump(std::ostringstream& s, int n) const override;
};

class NotExpressionNode : public ExpressionNode {
   public:
    std::unique_ptr<ExpressionNode> operand;
    NotExpressionNode(std::unique_ptr<ExpressionNode> operand, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class SymbolReferenceExpressionNode : public ExpressionNode {
   public:
    std::string name;
    const Node* boundSymbolDeclaration = nullptr;  //  set during symbol binding
    SymbolReferenceExpressionNode(std::string name, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool isSymbolReference() const override;
};

//
// statements
//

enum class StatementType {
    kAssign,
    kCall,
    kConst,
    kContinue,
    kDimList,
    kDimMap,
    kDim,
    kDo,
    kExit,
    kForEach,
    kFor,
    kGroup,
    kIf,
    kJoin,
    kRethrow,
    kReturn,
    kSelectCase,
    kSelect,
    kThrow,
    kTry,
    kWhile,
};

class StatementNode : public Node {
   public:
    const StatementType type;
    explicit StatementNode(StatementType type, Token token);
};

class AssignLocationSuffixNode : public Node {
   public:
    std::optional<std::string> name;
    std::unique_ptr<ExpressionNode> arrayIndex;  // may be null
    AssignLocationSuffixNode(std::string name, Token token);
    AssignLocationSuffixNode(std::unique_ptr<ExpressionNode> arrayIndex, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class AssignStatementNode : public StatementNode {
   public:
    std::string name;
    std::vector<std::unique_ptr<AssignLocationSuffixNode>> suffixes;
    std::unique_ptr<ExpressionNode> value;
    AssignStatementNode(
        std::string name,
        std::vector<std::unique_ptr<AssignLocationSuffixNode>> suffixes,
        std::unique_ptr<ExpressionNode> value,
        Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class BodyNode : public Node {
   public:
    std::vector<std::unique_ptr<StatementNode>> statements;
    BodyNode(std::vector<std::unique_ptr<StatementNode>> statements, Token token);
    void dump(std::ostringstream& s, int n) const override;
};

class CallStatementNode : public StatementNode {
   public:
    std::string name;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;
    CallStatementNode(std::string name, std::vector<std::unique_ptr<ExpressionNode>> arguments, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class ConstStatementNode : public StatementNode {
   public:
    std::string name;
    std::unique_ptr<ConstValueExpressionNode> value;
    ConstStatementNode(std::string name, std::unique_ptr<ConstValueExpressionNode> value, Token token);
    MemberType getMemberType() const override;
    void dump(std::ostringstream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
};

enum class ContinueScope {
    kDo,
    kFor,
    kWhile,
};

class ContinueStatementNode : public StatementNode {
   public:
    ContinueScope scope;
    ContinueStatementNode(ContinueScope scope, Token token);
    void dump(std::ostringstream& s, int n) const override;
};

class DimListStatementNode : public StatementNode {
   public:
    std::string name;
    std::unique_ptr<BodyNode> body;
    DimListStatementNode(std::string name, std::unique_ptr<BodyNode> body, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
};

class DimMapStatementNode : public StatementNode {
   public:
    std::string name;
    std::unique_ptr<BodyNode> body;
    DimMapStatementNode(std::string name, std::unique_ptr<BodyNode> body, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
};

class DimStatementNode : public StatementNode {
   public:
    std::string name;
    std::unique_ptr<TypeNode> type;         // may be null
    std::unique_ptr<ExpressionNode> value;  // may be null
    DimStatementNode(std::string name, std::unique_ptr<TypeNode> type, Token token);
    DimStatementNode(std::string name, std::unique_ptr<ExpressionNode> value, Token token);
    MemberType getMemberType() const override;
    void dump(std::ostringstream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
    TypeNode* getChildTypeNode() const override;
};

enum class DoConditionPosition {
    kBeforeBody,
    kAfterBody,
};

enum class DoConditionType {
    kWhile,
    kUntil,
};

class DoConditionNode : public Node {
   public:
    std::unique_ptr<ExpressionNode> condition;
    DoConditionType conditionType;
    DoConditionNode(std::unique_ptr<ExpressionNode> condition, DoConditionType conditionType, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class DoStatementNode : public StatementNode {
   public:
    std::unique_ptr<DoConditionNode> condition;
    DoConditionPosition conditionPosition;
    std::unique_ptr<BodyNode> body;
    DoStatementNode(
        std::unique_ptr<DoConditionNode> condition,
        DoConditionPosition conditionPosition,
        std::unique_ptr<BodyNode> body,
        Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

enum class ExitScope {
    kDo,
    kFor,
    kSelectCase,
    kTry,
    kWhile,
};

class ExitStatementNode : public StatementNode {
   public:
    ExitScope scope;
    ExitStatementNode(ExitScope scope, Token token);
    void dump(std::ostringstream& s, int n) const override;
};

class ForEachStatementNode : public StatementNode {
   public:
    std::string needleName;
    std::unique_ptr<ExpressionNode> haystack;
    std::unique_ptr<BodyNode> body;
    ForEachStatementNode(
        std::string needleName,
        std::unique_ptr<ExpressionNode> haystack,
        std::unique_ptr<BodyNode> body,
        Token token);
    void dump(std::ostringstream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class ForStepNode : public Node {
   public:
    std::optional<decimal::Decimal> stepImmediate;
    std::unique_ptr<SymbolReferenceExpressionNode> stepConstant;  // may be null
    ForStepNode(decimal::Decimal stepImmediate, Token token);
    ForStepNode(std::unique_ptr<SymbolReferenceExpressionNode> stepConstant, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class ForStatementNode : public StatementNode {
   public:
    std::string loopVariableName;
    std::unique_ptr<ExpressionNode> fromValue;
    std::unique_ptr<ExpressionNode> toValue;
    std::unique_ptr<ForStepNode> step;  // may be null
    std::unique_ptr<BodyNode> body;
    ForStatementNode(
        std::string loopVariableName,
        std::unique_ptr<ExpressionNode> fromValue,
        std::unique_ptr<ExpressionNode> toValue,
        std::unique_ptr<ForStepNode> step,
        std::unique_ptr<BodyNode> body,
        Token token);
    void dump(std::ostringstream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class GroupKeyNameNode : public Node {
   public:
    std::string name;
    GroupKeyNameNode(std::string name, Token token);
    void dump(std::ostringstream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
};

class GroupStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> itemExpression;
    std::unique_ptr<ExpressionNode> groupingExpression;
    std::string groupName;
    std::unique_ptr<GroupKeyNameNode> keyName;  // may be null
    std::unique_ptr<BodyNode> body;
    GroupStatementNode(
        std::unique_ptr<ExpressionNode> itemExpression,
        std::unique_ptr<ExpressionNode> groupingExpression,
        std::string groupName,
        std::unique_ptr<GroupKeyNameNode> keyName,
        std::unique_ptr<BodyNode> body,
        Token token);
    void dump(std::ostringstream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class ElseIfNode : public Node {
   public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BodyNode> body;
    ElseIfNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<BodyNode> body, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class IfStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BodyNode> body;
    std::vector<std::unique_ptr<ElseIfNode>> elseIfs;
    std::unique_ptr<BodyNode> elseBody;  // may be null
    IfStatementNode(
        std::unique_ptr<ExpressionNode> condition,
        std::unique_ptr<BodyNode> body,
        std::vector<std::unique_ptr<ElseIfNode>> elseIfs,
        std::unique_ptr<BodyNode> elseBody,
        Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class JoinStatementNode : public StatementNode {
   public:
    std::string needleName;
    std::unique_ptr<ExpressionNode> haystack;
    std::unique_ptr<ExpressionNode> joinExpression;
    std::unique_ptr<BodyNode> body;
    JoinStatementNode(
        std::string needleName,
        std::unique_ptr<ExpressionNode> haystack,
        std::unique_ptr<ExpressionNode> joinExpression,
        std::unique_ptr<BodyNode> body,
        Token token);
    void dump(std::ostringstream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class RethrowStatementNode : public StatementNode {
   public:
    explicit RethrowStatementNode(Token token);
    void dump(std::ostringstream& s, int n) const override;
};

class ReturnStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> expression;  // may be null
    ReturnStatementNode(std::unique_ptr<ExpressionNode> expression, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class CaseValueNode : public Node {
   public:
    std::unique_ptr<ExpressionNode> expression;
    std::unique_ptr<ExpressionNode> toExpression;  // may be null
    CaseValueNode(
        std::unique_ptr<ExpressionNode> expression,
        std::unique_ptr<ExpressionNode> toExpression,
        Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class CaseNode : public Node {
   public:
    std::vector<std::unique_ptr<CaseValueNode>> values;
    std::unique_ptr<BodyNode> body;
    CaseNode(std::vector<std::unique_ptr<CaseValueNode>> values, std::unique_ptr<BodyNode> body, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class SelectCaseStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> expression;
    std::vector<std::unique_ptr<CaseNode>> cases;
    SelectCaseStatementNode(
        std::unique_ptr<ExpressionNode> expression,
        std::vector<std::unique_ptr<CaseNode>> cases,
        Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class SelectStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> expression;
    std::unique_ptr<ExpressionNode> toExpression;  // may be null
    SelectStatementNode(
        std::unique_ptr<ExpressionNode> expression,
        std::unique_ptr<ExpressionNode> toExpression,
        Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class ThrowStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> expression;
    ThrowStatementNode(std::unique_ptr<ExpressionNode> expression, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

class TryStatementNode : public StatementNode {
   public:
    std::unique_ptr<BodyNode> tryBody;
    std::unique_ptr<BodyNode> catchBody;    // may be null
    std::unique_ptr<BodyNode> finallyBody;  // may be null
    TryStatementNode(
        std::unique_ptr<BodyNode> tryBody,
        std::unique_ptr<BodyNode> catchBody,
        std::unique_ptr<BodyNode> finallyBody,
        Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
};

class WhileStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BodyNode> body;
    WhileStatementNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<BodyNode> body, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(const VisitExpressionFunc& func) const override;
};

//
// members
//

class ParameterNode : public Node {
   public:
    std::string name;
    std::unique_ptr<TypeNode> type;
    ParameterNode(std::string name, std::unique_ptr<TypeNode> type, Token token);
    void dump(std::ostringstream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
};

class GlobalVariableNode : public Node {
   public:
    std::string name;
    size_t index;
    GlobalVariableNode(std::string name, size_t index);
    void dump(std::ostringstream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
};

class ProcedureNode : public Node {
   public:
    std::string name;
    std::vector<std::unique_ptr<ParameterNode>> parameters;
    std::unique_ptr<TypeNode> returnType;  // null for subroutines
    std::unique_ptr<BodyNode> body;
    std::vector<std::unique_ptr<GlobalVariableNode>> globalVariables;  // inserted after parsing
    ProcedureNode(
        std::string name,
        std::vector<std::unique_ptr<ParameterNode>> parameters,
        std::unique_ptr<TypeNode> returnType,
        std::unique_ptr<BodyNode> body,
        Token token);
    ProcedureNode(
        std::string name,
        std::vector<std::unique_ptr<ParameterNode>> parameters,
        std::unique_ptr<BodyNode> body,
        Token token);
    MemberType getMemberType() const override;
    void dump(std::ostringstream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
};

class TypeDeclarationNode : public Node {
   public:
    std::string name;
    std::vector<std::unique_ptr<ParameterNode>> fields;
    TypeDeclarationNode(std::string name, std::vector<std::unique_ptr<ParameterNode>> fields, Token token);
    MemberType getMemberType() const override;
    void dump(std::ostringstream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
};

class ProgramNode : public Node {
   public:
    std::vector<std::unique_ptr<Node>> members;
    ProgramNode(std::vector<std::unique_ptr<Node>> members, Token token);
    void dump(std::ostringstream& s, int n) const override;
};

}  // namespace basic
