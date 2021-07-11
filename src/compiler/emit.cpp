#include "emit.h"
#include "vm/Opcode.h"
#include "vm/systemCall.h"

using std::array;
using std::vector;
using vm::Opcode;
using vm::SystemCall;

namespace compiler {

class ProcedureState {
   public:
    vector<uint8_t> bytecode;

    template <typename TOutputInt, typename TInputInt>
    void emitInt(TInputInt value) {
        array<uint8_t, sizeof(TOutputInt)> bytes{};
        auto convertedValue = static_cast<TOutputInt>(value);
        memcpy(bytes.data(), &convertedValue, sizeof(TOutputInt));
        bytecode.insert(bytecode.end(), bytes.begin(), bytes.end());
    }

    void op(Opcode opcode) { emitInt<uint8_t>(opcode); }

    void syscall(Opcode opcode, SystemCall systemCall, uint8_t numVals, uint8_t numObjs) {
        op(opcode);
        emitInt<uint16_t>(systemCall);
        emitInt<uint8_t>(numVals);
        emitInt<uint8_t>(numObjs);
    }

    void pushString(const std::string& str) {
        op(Opcode::kPushImmediateUtf8);
        emitInt<uint32_t>(str.size());
        for (auto ch : str) {
            emitInt<uint8_t>(ch);
        }
    }
};

static void emitBinaryExpression(const BinaryExpressionNode& expressionNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitCallExpression(const CallExpressionNode& expressionNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitLiteralArrayExpression(const LiteralArrayExpressionNode& expressionNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitLiteralBooleanExpression(const LiteralBooleanExpressionNode& expressionNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitLiteralNumberExpression(const LiteralNumberExpressionNode& expressionNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitLiteralRecordExpression(const LiteralRecordExpressionNode& expressionNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitLiteralStringExpression(const LiteralStringExpressionNode& expressionNode, ProcedureState* state) {
    state->pushString(expressionNode.value);
}

static void emitConstValueExpressionNode(const ConstValueExpressionNode& expressionNode, ProcedureState* state) {
    switch (expressionNode.getConstValueExpressionType()) {
        case ConstValueExpressionType::kArray:
            emitLiteralArrayExpression(dynamic_cast<const LiteralArrayExpressionNode&>(expressionNode), state);
            break;
        case ConstValueExpressionType::kBoolean:
            emitLiteralBooleanExpression(dynamic_cast<const LiteralBooleanExpressionNode&>(expressionNode), state);
            break;
        case ConstValueExpressionType::kNumber:
            emitLiteralNumberExpression(dynamic_cast<const LiteralNumberExpressionNode&>(expressionNode), state);
            break;
        case ConstValueExpressionType::kRecord:
            emitLiteralRecordExpression(dynamic_cast<const LiteralRecordExpressionNode&>(expressionNode), state);
            break;
        case ConstValueExpressionType::kString:
            emitLiteralStringExpression(dynamic_cast<const LiteralStringExpressionNode&>(expressionNode), state);
            break;
        default:
            assert(false);
            throw std::runtime_error("Unexpected ConstValueExpressionType");
    }
}

static void emitConvertExpression(const ConvertExpressionNode& expressionNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitDottedExpression(const DottedExpressionNode& expressionNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitNotExpression(const NotExpressionNode& expressionNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitSymbolReferenceExpression(const SymbolReferenceExpressionNode& expressionNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitExpression(const ExpressionNode& expressionNode, ProcedureState* state) {
    switch (expressionNode.getExpressionType()) {
        case ExpressionType::kBinary:
            emitBinaryExpression(dynamic_cast<const BinaryExpressionNode&>(expressionNode), state);
            break;
        case ExpressionType::kCall:
            emitCallExpression(dynamic_cast<const CallExpressionNode&>(expressionNode), state);
            break;
        case ExpressionType::kConstValue:
            emitConstValueExpressionNode(dynamic_cast<const ConstValueExpressionNode&>(expressionNode), state);
            break;
        case ExpressionType::kConvert:
            emitConvertExpression(dynamic_cast<const ConvertExpressionNode&>(expressionNode), state);
            break;
        case ExpressionType::kDotted:
            emitDottedExpression(dynamic_cast<const DottedExpressionNode&>(expressionNode), state);
            break;
        case ExpressionType::kNot:
            emitNotExpression(dynamic_cast<const NotExpressionNode&>(expressionNode), state);
            break;
        case ExpressionType::kSymbolReference:
            emitSymbolReferenceExpression(dynamic_cast<const SymbolReferenceExpressionNode&>(expressionNode), state);
            break;
        default:
            assert(false);
            throw std::runtime_error("Unknown ExpressionType");
    }
}

static void emitAssignStatement(const AssignStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitCallStatement(const CallStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitConstStatement(const ConstStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitContinueStatement(const ContinueStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitDimListStatement(const DimListStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitDimMapStatement(const DimMapStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitDimStatement(const DimStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitDoStatement(const DoStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitExitStatement(const ExitStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitForEachStatement(const ForEachStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitForStatement(const ForStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitGroupStatement(const GroupStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitIfStatement(const IfStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitJoinStatement(const JoinStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitRethrowStatement(const RethrowStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitReturnStatement(const ReturnStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitSelectCaseStatement(const SelectCaseStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitSelectStatement(const SelectStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitThrowStatement(const ThrowStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitTryStatement(const TryStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitWhileStatement(const WhileStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitPrintStatement(const PrintStatementNode& statementNode, ProcedureState* state) {
    for (auto& expressionNode : statementNode.expressions) {
        emitExpression(*expressionNode, state);
        state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
    }

    if (!statementNode.trailingSemicolon) {
        state->pushString("\n");
        state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
    }

    state->syscall(Opcode::kSystemCall, SystemCall::kFlushConsoleOutput, 0, 0);
}

static void emitInputStatement(const InputStatementNode& statementNode, ProcedureState* state) {
    throw std::runtime_error("not impl");
}

static void emitStatement(const StatementNode& statementNode, ProcedureState* state) {
    switch (statementNode.getStatementType()) {
        case StatementType::kAssign:
            emitAssignStatement(dynamic_cast<const AssignStatementNode&>(statementNode), state);
            break;
        case StatementType::kCall:
            emitCallStatement(dynamic_cast<const CallStatementNode&>(statementNode), state);
            break;
        case StatementType::kConst:
            emitConstStatement(dynamic_cast<const ConstStatementNode&>(statementNode), state);
            break;
        case StatementType::kContinue:
            emitContinueStatement(dynamic_cast<const ContinueStatementNode&>(statementNode), state);
            break;
        case StatementType::kDimList:
            emitDimListStatement(dynamic_cast<const DimListStatementNode&>(statementNode), state);
            break;
        case StatementType::kDimMap:
            emitDimMapStatement(dynamic_cast<const DimMapStatementNode&>(statementNode), state);
            break;
        case StatementType::kDim:
            emitDimStatement(dynamic_cast<const DimStatementNode&>(statementNode), state);
            break;
        case StatementType::kDo:
            emitDoStatement(dynamic_cast<const DoStatementNode&>(statementNode), state);
            break;
        case StatementType::kExit:
            emitExitStatement(dynamic_cast<const ExitStatementNode&>(statementNode), state);
            break;
        case StatementType::kForEach:
            emitForEachStatement(dynamic_cast<const ForEachStatementNode&>(statementNode), state);
            break;
        case StatementType::kFor:
            emitForStatement(dynamic_cast<const ForStatementNode&>(statementNode), state);
            break;
        case StatementType::kGroup:
            emitGroupStatement(dynamic_cast<const GroupStatementNode&>(statementNode), state);
            break;
        case StatementType::kIf:
            emitIfStatement(dynamic_cast<const IfStatementNode&>(statementNode), state);
            break;
        case StatementType::kJoin:
            emitJoinStatement(dynamic_cast<const JoinStatementNode&>(statementNode), state);
            break;
        case StatementType::kRethrow:
            emitRethrowStatement(dynamic_cast<const RethrowStatementNode&>(statementNode), state);
            break;
        case StatementType::kReturn:
            emitReturnStatement(dynamic_cast<const ReturnStatementNode&>(statementNode), state);
            break;
        case StatementType::kSelectCase:
            emitSelectCaseStatement(dynamic_cast<const SelectCaseStatementNode&>(statementNode), state);
            break;
        case StatementType::kSelect:
            emitSelectStatement(dynamic_cast<const SelectStatementNode&>(statementNode), state);
            break;
        case StatementType::kThrow:
            emitThrowStatement(dynamic_cast<const ThrowStatementNode&>(statementNode), state);
            break;
        case StatementType::kTry:
            emitTryStatement(dynamic_cast<const TryStatementNode&>(statementNode), state);
            break;
        case StatementType::kWhile:
            emitWhileStatement(dynamic_cast<const WhileStatementNode&>(statementNode), state);
            break;
        case StatementType::kPrint:
            emitPrintStatement(dynamic_cast<const PrintStatementNode&>(statementNode), state);
            break;
        case StatementType::kInput:
            emitInputStatement(dynamic_cast<const InputStatementNode&>(statementNode), state);
            break;
        default:
            assert(false);
            throw std::runtime_error("Unknown StatementType.");
    }
}

static void emitBody(const BodyNode& bodyNode, ProcedureState* state) {
    auto debug_count = bodyNode.statements.size();
    (void)debug_count;
    for (const auto& statement : bodyNode.statements) {
        emitStatement(*statement, state);
    }
}

vector<uint8_t> emit(const ProcedureNode& procedureNode) {
    ProcedureState state;
    emitBody(*procedureNode.body, &state);
    state.op(Opcode::kReturn);
    return state.bytecode;
}

}  // namespace compiler
