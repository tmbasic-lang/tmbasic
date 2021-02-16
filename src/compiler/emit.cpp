#include "emit.h"

using std::vector;

namespace compiler {

class ProcedureState {
   public:
    // these include any parameters
    int numObjects = 0;
    int numValues = 0;
    vector<uint8_t> bytecode;
};

static void emitAssignStatement(const AssignStatementNode& statementNode, ProcedureState* state) {}

static void emitCallStatement(const CallStatementNode& statementNode, ProcedureState* state) {}

static void emitConstStatement(const ConstStatementNode& statementNode, ProcedureState* state) {}

static void emitContinueStatement(const ContinueStatementNode& statementNode, ProcedureState* state) {}

static void emitDimListStatement(const DimListStatementNode& statementNode, ProcedureState* state) {}

static void emitDimMapStatement(const DimMapStatementNode& statementNode, ProcedureState* state) {}

static void emitDimStatement(const DimStatementNode& statementNode, ProcedureState* state) {}

static void emitDoStatement(const DoStatementNode& statementNode, ProcedureState* state) {}

static void emitExitStatement(const ExitStatementNode& statementNode, ProcedureState* state) {}

static void emitForEachStatement(const ForEachStatementNode& statementNode, ProcedureState* state) {}

static void emitForStatement(const ForStatementNode& statementNode, ProcedureState* state) {}

static void emitGroupStatement(const GroupStatementNode& statementNode, ProcedureState* state) {}

static void emitIfStatement(const IfStatementNode& statementNode, ProcedureState* state) {}

static void emitJoinStatement(const JoinStatementNode& statementNode, ProcedureState* state) {}

static void emitRethrowStatement(const RethrowStatementNode& statementNode, ProcedureState* state) {}

static void emitReturnStatement(const ReturnStatementNode& statementNode, ProcedureState* state) {}

static void emitSelectCaseStatement(const SelectCaseStatementNode& statementNode, ProcedureState* state) {}

static void emitSelectStatement(const SelectStatementNode& statementNode, ProcedureState* state) {}

static void emitThrowStatement(const ThrowStatementNode& statementNode, ProcedureState* state) {}

static void emitTryStatement(const TryStatementNode& statementNode, ProcedureState* state) {}

static void emitWhileStatement(const WhileStatementNode& statementNode, ProcedureState* state) {}

static void emitStatement(const StatementNode& statementNode, ProcedureState* state) {
    switch (statementNode.type) {
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
        default:
            assert(false);
            break;
    }
}

static void emitBody(const BodyNode& bodyNode, ProcedureState* state) {
    for (const auto& statement : bodyNode.statements) {
        emitStatement(*statement, state);
    }
}

vector<uint8_t> emit(const ProcedureNode& procedureNode) {
    ProcedureState state;
    emitBody(*procedureNode.body, &state);
    return state.bytecode;
}

}  // namespace compiler
