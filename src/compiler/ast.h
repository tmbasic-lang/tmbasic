#pragma once

#include "../common.h"
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
class YieldStatementNode;

typedef std::function<bool(BodyNode*)> VisitBodyFunc;
typedef std::function<bool(ExpressionNode*)> VisitExpressionFunc;

class Node {
   public:
    Token token;

    explicit Node(Token token);
    virtual ~Node();
    virtual void dump(std::ostream& s, int n) const;
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

    // local/global variable symbol reference nodes
    std::optional<uint16_t> localValueIndex{};
    std::optional<uint16_t> localObjectIndex{};
    std::optional<uint16_t> globalValueIndex{};
    std::optional<uint16_t> globalObjectIndex{};

    // DimList, DimMap, Procedure
    virtual std::vector<YieldStatementNode*>* getYieldStatementNodesList();
};

//
// types
//

enum class Kind {
    kAny,
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
    std::string nameLowercase;
    boost::local_shared_ptr<TypeNode> type;

    // added during compilation
    std::optional<int> argumentValueIndex{};
    std::optional<int> argumentObjectIndex{};
    std::optional<int> fieldValueIndex{};
    std::optional<int> fieldObjectIndex{};

    ParameterNode(std::string name, boost::local_shared_ptr<TypeNode> type, Token token);
    void dump(std::ostream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
};

class TypeNode : public Node {
   public:
    Kind kind;
    std::optional<std::string> recordName;  // kind = kRecord (named)
    std::vector<boost::local_shared_ptr<ParameterNode>>
        fields;  // kind = kRecord (anonymous), will be filled for named types by bindNamedTypes
    // nullable type parameters
    boost::local_shared_ptr<TypeNode> listItemType;       // kind = kList, null for generic
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
    TypeNode(const TypeNode& source) = delete;
    void dump(std::ostream& s, int n) const override;
    bool isValueType() const;
    bool equals(const TypeNode& target) const;
    std::string toString() const;
};

//
// expressions
//

enum class ExpressionType {
    kBinary,
    kConstValue,
    kConvert,
    kDotted,
    kNot,
    kSymbolReference,
    kFunctionCall,
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
    kPower,
};

class BinaryExpressionSuffixNode : public Node {
   public:
    BinaryOperator binaryOperator;
    std::unique_ptr<ExpressionNode> rightOperand;
    BinaryExpressionSuffixNode(
        BinaryOperator binaryOperator,
        std::unique_ptr<ExpressionNode> rightOperand,
        Token token);
    void dump(std::ostream& s, int n) const override;
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
    void dump(std::ostream& s, int n) const override;
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
    void dump(std::ostream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    TypeNode* getChildTypeNode() const override;
    ExpressionType getExpressionType() const override;
};

class DottedExpressionSuffixNode : public Node {
   public:
    // it will be either a .name or a collection index/function call
    std::optional<std::string> name{};
    std::optional<std::string> nameLowercase{};
    std::vector<std::unique_ptr<ExpressionNode>> collectionIndexOrCallArgs{};

    // set by type checker
    ParameterNode* boundParameterNode{};
    std::optional<int> fieldValueIndex{};
    std::optional<int> fieldObjectIndex{};

    DottedExpressionSuffixNode(std::string name, Token token);
    DottedExpressionSuffixNode(std::vector<std::unique_ptr<ExpressionNode>> collectionIndexOrCallArgs, Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    inline bool isFieldAccess() { return name.has_value(); }
    inline bool isIndexOrCall() { return !name.has_value(); }
};

class DottedExpressionNode : public ExpressionNode {
   public:
    std::unique_ptr<ExpressionNode> base;
    std::vector<std::unique_ptr<DottedExpressionSuffixNode>> dottedSuffixes;
    DottedExpressionNode(
        std::unique_ptr<ExpressionNode> base,
        std::vector<std::unique_ptr<DottedExpressionSuffixNode>> dottedSuffixes,
        Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    ExpressionType getExpressionType() const override;
};

// fixDottedExpressionFunctionCalls() creates this node to be the base of DottedExpressionNodes.
class FunctionCallExpressionNode : public ExpressionNode {
   public:
    std::string name;
    std::vector<std::unique_ptr<ExpressionNode>> args;
    FunctionCallExpressionNode(std::string name, std::vector<std::unique_ptr<ExpressionNode>> args, Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    ExpressionType getExpressionType() const override;
};

class LiteralArrayExpressionNode : public ConstValueExpressionNode {
   public:
    std::vector<std::unique_ptr<ExpressionNode>> elements;
    LiteralArrayExpressionNode(std::vector<std::unique_ptr<ExpressionNode>> elements, Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    ConstValueExpressionType getConstValueExpressionType() const override;
};

class LiteralBooleanExpressionNode : public ConstValueExpressionNode {
   public:
    bool value;
    LiteralBooleanExpressionNode(bool value, Token token);
    void dump(std::ostream& s, int n) const override;
    ConstValueExpressionType getConstValueExpressionType() const override;
};

class LiteralNumberExpressionNode : public ConstValueExpressionNode {
   public:
    decimal::Decimal value;
    LiteralNumberExpressionNode(decimal::Decimal value, Token token);
    void dump(std::ostream& s, int n) const override;
    ConstValueExpressionType getConstValueExpressionType() const override;
};

class LiteralRecordFieldNode : public Node {
   public:
    std::string key;
    std::unique_ptr<ExpressionNode> value;
    LiteralRecordFieldNode(std::string key, std::unique_ptr<ExpressionNode> value, Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
};

class LiteralRecordExpressionNode : public ConstValueExpressionNode {
   public:
    std::vector<std::unique_ptr<LiteralRecordFieldNode>> fields;
    LiteralRecordExpressionNode(std::vector<std::unique_ptr<LiteralRecordFieldNode>>, Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    ConstValueExpressionType getConstValueExpressionType() const override;
};

class LiteralStringExpressionNode : public ConstValueExpressionNode {
   public:
    std::string value;
    LiteralStringExpressionNode(std::string value, Token token);
    void dump(std::ostream& s, int n) const override;
    ConstValueExpressionType getConstValueExpressionType() const override;
};

class NotExpressionNode : public ExpressionNode {
   public:
    std::unique_ptr<ExpressionNode> operand;
    NotExpressionNode(std::unique_ptr<ExpressionNode> operand, Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    ExpressionType getExpressionType() const override;
};

class SymbolReferenceExpressionNode : public ExpressionNode {
   public:
    std::string name;

    // added during compile
    const Node* boundSymbolDeclaration = nullptr;  // set during symbol binding

    SymbolReferenceExpressionNode(std::string name, Token token);
    void dump(std::ostream& s, int n) const override;
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
    kIf,
    kRethrow,
    kReturn,
    kSelectCase,
    kYield,
    kThrow,
    kTry,
    kWhile,
    kPrint,
    kInput,
};

class StatementNode : public Node {
   public:
    explicit StatementNode(Token token);

    // added during compilation
    std::optional<size_t> tempLocalValueIndex{};
    std::optional<size_t> tempLocalObjectIndex{};

    virtual StatementType getStatementType() const = 0;
    virtual int getTempLocalValueCount() const;
    virtual int getTempLocalObjectCount() const;
};

class AssignLocationSuffixNode : public Node {
   public:
    std::optional<std::string> name;
    std::unique_ptr<ExpressionNode> arrayIndex;  // may be null
    AssignLocationSuffixNode(std::string name, Token token);
    AssignLocationSuffixNode(std::unique_ptr<ExpressionNode> arrayIndex, Token token);
    void dump(std::ostream& s, int n) const override;
};

class AssignStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> target;
    std::unique_ptr<ExpressionNode> value;
    AssignStatementNode(std::unique_ptr<ExpressionNode> target, std::unique_ptr<ExpressionNode> value, Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
};

class BodyNode : public Node {
   public:
    std::vector<std::unique_ptr<StatementNode>> statements;
    BodyNode(std::vector<std::unique_ptr<StatementNode>> statements, Token token);
    void dump(std::ostream& s, int n) const override;
};

class CallStatementNode : public StatementNode {
   public:
    std::string name;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;
    CallStatementNode(std::string name, std::vector<std::unique_ptr<ExpressionNode>> arguments, Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
};

class ConstStatementNode : public StatementNode {
   public:
    std::string name;
    std::unique_ptr<ConstValueExpressionNode> value;
    ConstStatementNode(std::string name, std::unique_ptr<ConstValueExpressionNode> value, Token token);
    MemberType getMemberType() const override;
    void dump(std::ostream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
    bool isSymbolVisibleToSiblingStatements() const override;
    StatementType getStatementType() const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
};

enum class LoopType {
    kDo,
    kFor,
    kWhile,
};

class ContinueStatementNode : public StatementNode {
   public:
    LoopType scope;
    ContinueStatementNode(LoopType scope, Token token);
    void dump(std::ostream& s, int n) const override;
    StatementType getStatementType() const override;
};

class YieldStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> expression;
    std::unique_ptr<ExpressionNode> toExpression;  // may be null

    // added by bindYieldStatements
    Node* boundCollectionDeclaration{};  // DimListNode | DimMapNode | ProcedureNode

    YieldStatementNode(
        std::unique_ptr<ExpressionNode> expression,
        std::unique_ptr<ExpressionNode> toExpression,
        Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
};

class DimListStatementNode : public StatementNode {
   public:
    std::string name;
    std::unique_ptr<BodyNode> body;

    // added by bindYieldStatements
    std::vector<YieldStatementNode*> yieldStatements;
    std::vector<YieldStatementNode*>* getYieldStatementNodesList() override;

    DimListStatementNode(std::string name, std::unique_ptr<BodyNode> body, Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    StatementType getStatementType() const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
    bool isSymbolVisibleToSiblingStatements() const override;
};

class DimMapStatementNode : public StatementNode {
   public:
    std::string name;
    std::unique_ptr<BodyNode> body;

    // added by bindYieldStatements
    std::vector<YieldStatementNode*> yieldStatements;
    std::vector<YieldStatementNode*>* getYieldStatementNodesList() override;

    DimMapStatementNode(std::string name, std::unique_ptr<BodyNode> body, Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    StatementType getStatementType() const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
    bool isSymbolVisibleToSiblingStatements() const override;
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
    void dump(std::ostream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
    bool isSymbolVisibleToSiblingStatements() const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    TypeNode* getChildTypeNode() const override;
    StatementType getStatementType() const override;
};

class DoStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BodyNode> body;
    DoStatementNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<BodyNode> body, Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
};

class ExitStatementNode : public StatementNode {
   public:
    LoopType scope;
    ExitStatementNode(LoopType scope, Token token);
    void dump(std::ostream& s, int n) const override;
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
    void dump(std::ostream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
    int getTempLocalValueCount() const override;
    int getTempLocalObjectCount() const override;
};

class ForStatementNode : public StatementNode {
   public:
    std::string loopVariableName;
    std::unique_ptr<ExpressionNode> fromValue;
    std::unique_ptr<ExpressionNode> toValue;
    std::unique_ptr<ExpressionNode> step;  // may be null
    std::unique_ptr<BodyNode> body;
    ForStatementNode(
        std::string loopVariableName,
        std::unique_ptr<ExpressionNode> fromValue,
        std::unique_ptr<ExpressionNode> toValue,
        std::unique_ptr<ExpressionNode> step,
        std::unique_ptr<BodyNode> body,
        Token token);
    void dump(std::ostream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
    int getTempLocalValueCount() const override;

   private:
    mutable boost::local_shared_ptr<TypeNode> _type = nullptr;  // set by getSymbolDeclarationType(); always Number
};

class ElseIfNode : public Node {
   public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BodyNode> body;
    ElseIfNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<BodyNode> body, Token token);
    void dump(std::ostream& s, int n) const override;
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
    void dump(std::ostream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
};

class RethrowStatementNode : public StatementNode {
   public:
    explicit RethrowStatementNode(Token token);
    void dump(std::ostream& s, int n) const override;
    StatementType getStatementType() const override;
};

class ReturnStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> expression;  // may be null
    ReturnStatementNode(std::unique_ptr<ExpressionNode> expression, Token token);
    void dump(std::ostream& s, int n) const override;
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
    void dump(std::ostream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
};

class CaseNode : public Node {
   public:
    std::vector<std::unique_ptr<CaseValueNode>> values;
    std::unique_ptr<BodyNode> body;
    CaseNode(std::vector<std::unique_ptr<CaseValueNode>> values, std::unique_ptr<BodyNode> body, Token token);
    void dump(std::ostream& s, int n) const override;
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
    void dump(std::ostream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
    int getTempLocalValueCount() const override;
    int getTempLocalObjectCount() const override;
};

class ThrowStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> message;
    std::unique_ptr<ExpressionNode> code;  // may be null
    ThrowStatementNode(std::unique_ptr<ExpressionNode> message, std::unique_ptr<ExpressionNode> code, Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
};

class TryStatementNode : public StatementNode {
   public:
    std::unique_ptr<BodyNode> tryBody;
    std::unique_ptr<BodyNode> catchBody;
    TryStatementNode(std::unique_ptr<BodyNode> tryBody, std::unique_ptr<BodyNode> catchBody, Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    StatementType getStatementType() const override;
};

class WhileStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BodyNode> body;
    WhileStatementNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<BodyNode> body, Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitBodies(const VisitBodyFunc& func) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
};

class PrintStatementNode : public StatementNode {
   public:
    std::vector<std::unique_ptr<ExpressionNode>> expressions;
    bool trailingSemicolon;
    PrintStatementNode(std::vector<std::unique_ptr<ExpressionNode>> expressions, bool trailingSemicolon, Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
};

class InputStatementNode : public StatementNode {
   public:
    std::unique_ptr<ExpressionNode> target;
    InputStatementNode(std::unique_ptr<ExpressionNode> target, Token token);
    void dump(std::ostream& s, int n) const override;
    bool visitExpressions(bool rootsOnly, const VisitExpressionFunc& func) const override;
    StatementType getStatementType() const override;
};

//
// members
//

class GlobalVariableNode : public Node {
   public:
    std::string name;
    GlobalVariableNode(std::string name);
    void dump(std::ostream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
    boost::local_shared_ptr<TypeNode> getSymbolDeclarationType() const override;
};

class ProcedureNode : public Node {
   public:
    std::string name;
    std::vector<std::unique_ptr<ParameterNode>> parameters;
    boost::local_shared_ptr<TypeNode> returnType;  // null for subroutines
    std::unique_ptr<BodyNode> body;                // null for system procedures

    // added by bindYieldStatements
    std::vector<YieldStatementNode*> yieldStatements;
    std::vector<YieldStatementNode*>* getYieldStatementNodesList() override;

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
    void dump(std::ostream& s, int n) const override;
    std::optional<std::string> getSymbolDeclaration() const override;
};

class TypeDeclarationNode : public Node {
   public:
    std::string name;
    std::vector<std::unique_ptr<ParameterNode>> fields;
    TypeDeclarationNode(std::string name, std::vector<std::unique_ptr<ParameterNode>> fields, Token token);
    MemberType getMemberType() const override;
    void dump(std::ostream& s, int n) const override;
};

class ProgramNode : public Node {
   public:
    std::vector<std::unique_ptr<Node>> members;
    ProgramNode(std::vector<std::unique_ptr<Node>> members, Token token);
    void dump(std::ostream& s, int n) const override;
};

}  // namespace compiler
