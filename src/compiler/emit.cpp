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

static void emitAssignStatement(const basic::AssignStatementNode& statementNode, ProcedureState* state) {}
static void emitCallStatement(const basic::CallStatementNode& statementNode, ProcedureState* state) {}
static void emitConstStatement(const basic::ConstStatementNode& statementNode, ProcedureState* state) {}
static void emitContinueStatement(const basic::ContinueStatementNode& statementNode, ProcedureState* state) {}
static void emitDimListStatement(const basic::DimListStatementNode& statementNode, ProcedureState* state) {}
static void emitDimMapStatement(const basic::DimMapStatementNode& statementNode, ProcedureState* state) {}
static void emitDimStatement(const basic::DimStatementNode& statementNode, ProcedureState* state) {}
static void emitDimCollectionStatement(const basic::DimCollectionStatementNode& statementNode, ProcedureState* state) {}
static void emitDoStatement(const basic::DoStatementNode& statementNode, ProcedureState* state) {}
static void emitExitStatement(const basic::ExitStatementNode& statementNode, ProcedureState* state) {}
static void emitForEachStatement(const basic::ForEachStatementNode& statementNode, ProcedureState* state) {}
static void emitForStatement(const basic::ForStatementNode& statementNode, ProcedureState* state) {}
static void emitGroupStatement(const basic::GroupStatementNode& statementNode, ProcedureState* state) {}
static void emitIfStatement(const basic::IfStatementNode& statementNode, ProcedureState* state) {}
static void emitJoinStatement(const basic::JoinStatementNode& statementNode, ProcedureState* state) {}
static void emitRethrowStatement(const basic::RethrowStatementNode& statementNode, ProcedureState* state) {}
static void emitReturnStatement(const basic::ReturnStatementNode& statementNode, ProcedureState* state) {}
static void emitSelectCaseStatement(const basic::SelectCaseStatementNode& statementNode, ProcedureState* state) {}
static void emitSelectStatement(const basic::SelectStatementNode& statementNode, ProcedureState* state) {}
static void emitThrowStatement(const basic::ThrowStatementNode& statementNode, ProcedureState* state) {}
static void emitTryStatement(const basic::TryStatementNode& statementNode, ProcedureState* state) {}
static void emitWhileStatement(const basic::WhileStatementNode& statementNode, ProcedureState* state) {}

static void emitStatement(const basic::StatementNode& statementNode, ProcedureState* state) {
    switch (statementNode.type) {
        case basic::StatementType::kAssign:
            emitAssignStatement(dynamic_cast<const basic::AssignStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kCall:
            emitCallStatement(dynamic_cast<const basic::CallStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kConst:
            emitConstStatement(dynamic_cast<const basic::ConstStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kContinue:
            emitContinueStatement(dynamic_cast<const basic::ContinueStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kDimList:
            emitDimListStatement(dynamic_cast<const basic::DimListStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kDimMap:
            emitDimMapStatement(dynamic_cast<const basic::DimMapStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kDim:
            emitDimStatement(dynamic_cast<const basic::DimStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kDimCollection:
            emitDimCollectionStatement(dynamic_cast<const basic::DimCollectionStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kDo:
            emitDoStatement(dynamic_cast<const basic::DoStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kExit:
            emitExitStatement(dynamic_cast<const basic::ExitStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kForEach:
            emitForEachStatement(dynamic_cast<const basic::ForEachStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kFor:
            emitForStatement(dynamic_cast<const basic::ForStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kGroup:
            emitGroupStatement(dynamic_cast<const basic::GroupStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kIf:
            emitIfStatement(dynamic_cast<const basic::IfStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kJoin:
            emitJoinStatement(dynamic_cast<const basic::JoinStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kRethrow:
            emitRethrowStatement(dynamic_cast<const basic::RethrowStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kReturn:
            emitReturnStatement(dynamic_cast<const basic::ReturnStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kSelectCase:
            emitSelectCaseStatement(dynamic_cast<const basic::SelectCaseStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kSelect:
            emitSelectStatement(dynamic_cast<const basic::SelectStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kThrow:
            emitThrowStatement(dynamic_cast<const basic::ThrowStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kTry:
            emitTryStatement(dynamic_cast<const basic::TryStatementNode&>(statementNode), state);
            break;
        case basic::StatementType::kWhile:
            emitWhileStatement(dynamic_cast<const basic::WhileStatementNode&>(statementNode), state);
            break;
    }
}

static void emitBody(const basic::BodyNode& bodyNode, ProcedureState* state) {
    for (auto& statement : bodyNode.statements) {
        emitStatement(*statement, state);
    }
}

vector<uint8_t> emit(const basic::ProcedureNode& procedureNode) {
    ProcedureState state;
    emitBody(*procedureNode.body, &state);
    return state.bytecode;
}

}  // namespace compiler
