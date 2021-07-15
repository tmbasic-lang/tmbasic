#pragma once

#include "../common.h"
#include "compiler/CompiledProgram.h"
#include "compiler/Token.h"
#include "vm/systemCall.h"

namespace compiler {

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

typedef std::function<bool(BodyNode*)> VisitBodyFunc;
typedef std::function<bool(ExpressionNode*)> VisitExpressionFunc;

class Node {
   public:
    Token token;

    explicit Node(Token token);
    virtual ~Node();
    virtual void dump(std::ostringstream& s, int n) const;
    virtual MemberType getMemberType() const;
    virtual bool visitBodies(const VisitBodyFunc& func) const;
    virtual bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const;
    virtual bool isSymbolReference() const;
    virtual TypeNode* getChildTypeNode() const;  // statements and expressions

    // symbol declaration nodes
    virtual std::optional<std::string> getSymbolDeclaration() const;
    virtual boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const;
    virtual Node* getChildSymbolDeclaration() const;  // a sub-node that declares another symbol
    virtual bool isSymbolVisibleToSiblingStatements() const;

    // procedure nodes and call nodes
    std::optional<size_t> procedureIndex{};
    std::optional<vm::SystemCall> systemCall{};

    // symbol reference and procedure call nodes
    boost::local_shared_ptr<TypeNode> evaluatedType = nullptr;  // set during type checking

    // local variable symbol reference nodes
    std::optional<int> localValueIndex{};
    std::optional<int> localObjectIndex{};
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
    kTimeZone,
    kString,
    kList,
    kMap,
    kRecord,
    kOptional,
};

class ParameterNode : public Node {
   public:
    std::string name;
    boost::local_shared_ptr<TypeNode> type;

    // added during compilation
    std::optional<int> argumentValueIndex{};
    std::optional<int> argumentObjectIndex{};

    ParameterNode(std::string name, boost::local_shared_ptr<TypeNode> type, Token token);
    void dump(std::ostringstream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
};

class TypeNode : public Node {
   public:
    Kind kind;
    std::optional<std::string> recordName;              // kind = kRecord (named)
    std::optional<std::string> genericPlaceholderName;  // kind = kGenericPlaceholder
    std::vector<boost::local_shared_ptr<ParameterNode>>
        fields;  // kind = kRecord (anonymous), will be filled for named types by bindNamedTypes
    // nullable type parameters
    boost::local_shared_ptr<TypeNode> listItemType;       // kind = kList
    boost::local_shared_ptr<TypeNode> mapKeyType;         // kind = kMap
    boost::local_shared_ptr<TypeNode> mapValueType;       // kind = kMap
    boost::local_shared_ptr<TypeNode> optionalValueType;  // kind = kOptional
    bool namedTypesAreBound = false;                      // set by bindNamedTypes
    TypeNode(Kind kind, Token token);
    TypeNode(Kind kind, Token token, std::string recordName);
    TypeNode(Kind kind, Token token, boost::local_shared_ptr<TypeNode> optionalValueTypeOrListItemType);
    TypeNode(
        Kind kind,
        Token token,
        boost::local_shared_ptr<TypeNode> mapKeyType,
        boost::local_shared_ptr<TypeNode> mapValueType);
    TypeNode(Kind kind, Token token, std::vector<boost::local_shared_ptr<ParameterNode>> fields);
    TypeNode(const TypeNode& source);
    void dump(std::ostringstream& s, int n) const override;
    bool isValueType() const;
    bool canImplicitlyConvertTo(const TypeNode& target) const;
    bool isIdentical(const TypeNode& target) const;
};

//
// expressions
//

enum class ExpressionType {
    kBinary,
    kCall,
    kConstValue,
    kConvert,
    kDotted,
    kNot,
    kSymbolReference,
};

class ExpressionNode : public Node {
   public:
    explicit ExpressionNode(Token token);
    virtual ExpressionType getExpressionType() const = 0;
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
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
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
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    ExpressionType getExpressionType() const override;
};

class CallExpressionNode : public ExpressionNode {
   public:
    std::string name;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;
    CallExpressionNode(std::string name, std::vector<std::unique_ptr<ExpressionNode>> arguments, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    ExpressionType getExpressionType() const override;
};

enum class ConstValueExpressionType {
    kArray,
    kBoolean,
    kNumber,
    kRecord,
    kString,
};

class ConstValueExpressionNode : public ExpressionNode {
   public:
    explicit ConstValueExpressionNode(Token token);
    ExpressionType getExpressionType() const override;
    virtual ConstValueExpressionType getConstValueExpressionType() const = 0;
};

class ConvertExpressionNode : public ExpressionNode {
   public:
    std::unique_ptr<ExpressionNode> value;
    boost::local_shared_ptr<TypeNode> type;
    ConvertExpressionNode(std::unique_ptr<ExpressionNode> value, boost::local_shared_ptr<TypeNode> type, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    TypeNode* getChildTypeNode() const override;
    ExpressionType getExpressionType() const override;
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
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
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
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    ExpressionType getExpressionType() const override;
};

class LiteralArrayExpressionNode : public ConstValueExpressionNode {
   public:
    std::vector<std::unique_ptr<ExpressionNode>> elements;
    LiteralArrayExpressionNode(std::vector<std::unique_ptr<ExpressionNode>> elements, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    ConstValueExpressionType getConstValueExpressionType() const override;
};

class LiteralBooleanExpressionNode : public ConstValueExpressionNode {
   public:
    bool value;
    LiteralBooleanExpressionNode(bool value, Token token);
    void dump(std::ostringstream& s, int n) const override;
    ConstValueExpressionType getConstValueExpressionType() const override;
};

class LiteralNumberExpressionNode : public ConstValueExpressionNode {
   public:
    decimal::Decimal value;
    LiteralNumberExpressionNode(decimal::Decimal value, Token token);
    void dump(std::ostringstream& s, int n) const override;
    ConstValueExpressionType getConstValueExpressionType() const override;
};

class LiteralRecordFieldNode : public Node {
   public:
    std::string key;
    std::unique_ptr<ExpressionNode> value;
    LiteralRecordFieldNode(std::string key, std::unique_ptr<ExpressionNode> value, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
};

class LiteralRecordExpressionNode : public ConstValueExpressionNode {
   public:
    std::vector<std::unique_ptr<LiteralRecordFieldNode>> fields;
    LiteralRecordExpressionNode(std::vector<std::unique_ptr<LiteralRecordFieldNode>>, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    ConstValueExpressionType getConstValueExpressionType() const override;
};

class LiteralStringExpressionNode : public ConstValueExpressionNode {
   public:
    std::string value;
    LiteralStringExpressionNode(std::string value, Token token);
    void dump(std::ostringstream& s, int n) const override;
    ConstValueExpressionType getConstValueExpressionType() const override;
};

class NotExpressionNode : public ExpressionNode {
   public:
    std::unique_ptr<ExpressionNode> operand;
    NotExpressionNode(std::unique_ptr<ExpressionNode> operand, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    ExpressionType getExpressionType() const override;
};

class SymbolReferenceExpressionNode : public ExpressionNode {
   public:
    std::string name;

    // added during compile
    const Node* boundSymbolDeclaration = nullptr;  // set during symbol binding

    SymbolReferenceExpressionNode(std::string name, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool isSymbolReference() const override;
    ExpressionType getExpressionType() const override;
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
    kPrint,
    kInput,
};

class StatementNode : public Node {
   public:
    explicit StatementNode(Token token);
    virtual StatementType getStatementType() const = 0;
};

class AssignLocationSuffixNode : public Node {
   public:
    std::optional<std::string> name;
    std::unique_ptr<ExpressionNode> arrayIndex;  // may be null
    AssignLocationSuffixNode(std::string name, Token token);
    AssignLocationSuffixNode(std::unique_ptr<ExpressionNode> arrayIndex, Token token);
    void dump(std::ostringstream& s, int n) const override;
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
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
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
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
};

class ConstStatementNode : public StatementNode {
   public:
    std::string name;
    std::unique_ptr<ConstValueExpressionNode> value;
    ConstStatementNode(std::string name, std::unique_ptr<ConstValueExpressionNode> value, Token token);
    MemberType getMemberType() const override;
    void dump(std::ostringstream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
    StatementType getStatementType() const override;
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
    StatementType getStatementType() const override;
};

class DimListStatementNode : public StatementNode {
   public:
    std::string name;
    std::unique_ptr<BodyNode> body;
    DimListStatementNode(std::string name, std::unique_ptr<BodyNode> body, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    StatementType getStatementType() const override;
};

class DimMapStatementNode : public StatementNode {
   public:
    std::string name;
    std::unique_ptr<BodyNode> body;
    DimMapStatementNode(std::string name, std::unique_ptr<BodyNode> body, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    StatementType getStatementType() const override;
};

class DimStatementNode : public StatementNode {
   public:
    std::string name;
    boost::local_shared_ptr<TypeNode> type;  // may be null
    std::unique_ptr<ExpressionNode> value;   // may be null
    bool shared;
    DimStatementNode(std::string name, boost::local_shared_ptr<TypeNode> type, Token token, bool shared);
    DimStatementNode(std::string name, std::unique_ptr<ExpressionNode> value, Token token, bool shared);
    MemberType getMemberType() const override;
    void dump(std::ostringstream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
    bool isSymbolVisibleToSiblingStatements() const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    TypeNode* getChildTypeNode() const override;
    StatementType getStatementType() const override;
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
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
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
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
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
    StatementType getStatementType() const override;
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
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
};

class ForStepNode : public Node {
   public:
    std::optional<decimal::Decimal> stepImmediate;
    std::unique_ptr<SymbolReferenceExpressionNode> stepConstant;  // may be null
    ForStepNode(decimal::Decimal stepImmediate, Token token);
    ForStepNode(std::unique_ptr<SymbolReferenceExpressionNode> stepConstant, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
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
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;

   private:
    mutable boost::local_shared_ptr<TypeNode> _type = nullptr;  // set by getSymbolDeclarationType(); always Number
};

class GroupKeyNameNode : public Node {
   public:
    std::string name;
    GroupKeyNameNode(std::string name, Token token);
    void dump(std::ostringstream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
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
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
};

class ElseIfNode : public Node {
   public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BodyNode> body;
    ElseIfNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<BodyNode> body, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
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
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
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
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
};

class RethrowStatementNode : public StatementNode {
   public:
    explicit RethrowStatementNode(Token token);
    void dump(std::ostringstream& s, int n) const override;
    StatementType getStatementType() const override;
};

class ReturnStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> expression;  // may be null
    ReturnStatementNode(std::unique_ptr<ExpressionNode> expression, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
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
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
};

class CaseNode : public Node {
   public:
    std::vector<std::unique_ptr<CaseValueNode>> values;
    std::unique_ptr<BodyNode> body;
    CaseNode(std::vector<std::unique_ptr<CaseValueNode>> values, std::unique_ptr<BodyNode> body, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
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
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
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
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
};

class ThrowStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> expression;
    ThrowStatementNode(std::unique_ptr<ExpressionNode> expression, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
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
    StatementType getStatementType() const override;
};

class WhileStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BodyNode> body;
    WhileStatementNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<BodyNode> body, Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
};

class PrintStatementNode : public StatementNode {
   public:
    std::vector<std::unique_ptr<ExpressionNode>> expressions;
    std::optional<std::string> toIdentifier;
    bool trailingSemicolon;
    PrintStatementNode(
        std::vector<std::unique_ptr<ExpressionNode>> expressions,
        std::optional<std::string> toIdentifier,
        bool trailingSemicolon,
        Token token);
    void dump(std::ostringstream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
};

class InputStatementNode : public StatementNode {
   public:
    std::optional<std::string> fromIdentifier;
    std::string toIdentifier;
    InputStatementNode(std::optional<std::string> fromIdentifier, std::string toIdentifier, Token token);
    void dump(std::ostringstream& s, int n) const override;
    StatementType getStatementType() const override;
};

//
// members
//

class GlobalVariableNode : public Node {
   public:
    std::string name;
    GlobalVariableNode(std::string name);
    void dump(std::ostringstream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
};

class ProcedureNode : public Node {
   public:
    std::string name;
    std::vector<std::unique_ptr<ParameterNode>> parameters;
    boost::local_shared_ptr<TypeNode> returnType;  // null for subroutines
    std::unique_ptr<BodyNode> body;                // null for system procedures
    ProcedureNode(
        std::string name,
        std::vector<std::unique_ptr<ParameterNode>> parameters,
        boost::local_shared_ptr<TypeNode> returnType,
        std::unique_ptr<BodyNode> body,
        Token token);
    ProcedureNode(
        std::string name,
        std::vector<std::unique_ptr<ParameterNode>> parameters,
        std::unique_ptr<BodyNode> body,
        Token token);
    ProcedureNode(std::string name, std::vector<std::unique_ptr<ParameterNode>> parameters, vm::SystemCall systemCall);
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
};

class ProgramNode : public Node {
   public:
    std::vector<std::unique_ptr<Node>> members;
    ProgramNode(std::vector<std::unique_ptr<Node>> members, Token token);
    void dump(std::ostringstream& s, int n) const override;
};

}  // namespace compiler
