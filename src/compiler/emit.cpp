// uncomment to dump the generated assembly to std::cerr
//#define DUMP_ASM

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
    void emitInt(TInputInt value, bool log = true) {
        array<uint8_t, sizeof(TOutputInt)> bytes{};
        auto convertedValue = static_cast<TOutputInt>(value);
        memcpy(bytes.data(), &convertedValue, sizeof(TOutputInt));
        bytecode.insert(bytecode.end(), bytes.begin(), bytes.end());
        (void)log;
#ifdef DUMP_ASM
        if (log) {
            std::cerr << " " << static_cast<int>(value);
        }
#endif
    }

    void op(Opcode opcode, bool log = true) {
        bytecode.push_back(static_cast<uint8_t>(opcode));
        (void)log;
#ifdef DUMP_ASM
        if (log) {
            std::cerr << std::endl << NAMEOF_ENUM(opcode);
        }
#endif
    }

    void syscall(Opcode opcode, SystemCall systemCall, uint8_t numVals, uint8_t numObjs) {
        op(opcode, false);
        emitInt<uint16_t>(systemCall, false);
        emitInt<uint8_t>(numVals, false);
        emitInt<uint8_t>(numObjs, false);
#ifdef DUMP_ASM
        std::cerr << std::endl
                  << NAMEOF_ENUM(opcode) << " " << NAMEOF_ENUM(systemCall) << " " << static_cast<int>(numVals) << " "
                  << static_cast<int>(numObjs);
#endif
    }

    void pushString(const std::string& str) {
        op(Opcode::kPushImmediateUtf8, false);
        emitInt<uint32_t>(str.size(), false);
        for (auto ch : str) {
            emitInt<uint8_t>(ch, false);
        }
#ifdef DUMP_ASM
        std::cerr << std::endl << NAMEOF_ENUM(Opcode::kPushImmediateUtf8) << " \"" << str << "\"";
#endif
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
    state->op(Opcode::kPushImmediateDec128);
    auto triple = expressionNode.value.as_uint128_triple();
    state->emitInt<uint8_t>(triple.tag);
    state->emitInt<uint8_t>(triple.sign);
    state->emitInt<uint64_t>(triple.hi);
    state->emitInt<uint64_t>(triple.lo);
    state->emitInt<int64_t>(triple.exp);
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
    assert(expressionNode.boundSymbolDeclaration != nullptr);
    auto* decl = expressionNode.boundSymbolDeclaration;
    if (decl->localValueIndex.has_value()) {
        state->op(Opcode::kPushLocalValue);
        state->emitInt<uint16_t>(*decl->localValueIndex);
    } else if (decl->localObjectIndex.has_value()) {
        state->op(Opcode::kPushLocalObject);
        state->emitInt<uint16_t>(*decl->localObjectIndex);
    } else {
        throw std::runtime_error("not impl");
    }
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
    const auto& type = *statementNode.evaluatedType;

    // if an initial value is provided, then use that
    if (statementNode.value != nullptr) {
        emitExpression(*statementNode.value, state);
        if (statementNode.localValueIndex.has_value()) {
            state->op(Opcode::kSetLocalValue);
            state->emitInt<uint16_t>(*statementNode.localValueIndex);
        } else if (statementNode.localObjectIndex.has_value()) {
            state->op(Opcode::kSetLocalObject);
            state->emitInt<uint16_t>(*statementNode.localObjectIndex);
        } else {
            throw std::runtime_error("Internal error. No local variable index found!");
        }
        return;
    }

    // initialize all value types to 0.
    if (type.isValueType()) {
        assert(statementNode.localValueIndex >= 0);
        state->op(Opcode::kPushImmediateInt64);
        state->emitInt<int64_t>(0);
        state->op(Opcode::kSetLocalValue);
        state->emitInt<uint16_t>(*statementNode.localValueIndex);
        return;
    }

    // on the other hand, object types must be created with special code per-type.
    // in the future we should optimize this by creating a single empty object of each type and reusing them.
    // this would be safe because our data types are all immutable.
    switch (type.kind) {
        case Kind::kBoolean:
        case Kind::kDate:
        case Kind::kDateTime:
        case Kind::kNumber:
        case Kind::kTimeSpan:
            // handled above already
            assert(false);
            break;

        // a DateTimeOffset is a record with two values
        case Kind::kDateTimeOffset:
            state->op(Opcode::kPushImmediateInt64);
            state->emitInt<int64_t>(0);
            state->op(Opcode::kPushImmediateInt64);
            state->emitInt<int64_t>(0);
            state->op(Opcode::kRecordNew);
            state->emitInt<uint16_t>(2);
            state->emitInt<uint16_t>(0);
            break;

        case Kind::kList:
            state->op(type.listItemType->isValueType() ? Opcode::kValueListNew : Opcode::kObjectListNew);
            state->emitInt<uint16_t>(0);
            break;

        case Kind::kMap: {
            auto keyV = type.mapKeyType->isValueType();
            auto keyO = !keyV;
            auto valueV = type.mapValueType->isValueType();
            auto valueO = !valueV;
            SystemCall systemCallNumber{};
            if (keyV && valueV) {
                systemCallNumber = SystemCall::kValueToValueMapNew;
            } else if (keyV && valueO) {
                systemCallNumber = SystemCall::kValueToObjectMapNew;
            } else if (keyO && valueV) {
                systemCallNumber = SystemCall::kObjectToValueMapNew;
            } else if (keyO && valueO) {
                systemCallNumber = SystemCall::kObjectToObjectMapNew;
            }
            state->syscall(Opcode::kSystemCallO, systemCallNumber, 0, 0);
            break;
        }

        case Kind::kOptional:
            state->syscall(
                Opcode::kSystemCallO,
                type.optionalValueType->isValueType() ? SystemCall::kValueOptionalNewMissing
                                                      : SystemCall::kObjectOptionalNewMissing,
                0, 0);
            break;

        case Kind::kRecord:
            throw std::runtime_error("not impl");

        case Kind::kString:
            state->pushString("");
            break;

        case Kind::kTimeZone:
            state->pushString("UTC");
            state->syscall(Opcode::kSystemCallO, SystemCall::kTimeZoneFromName, 0, 1);
            break;

        default:
            throw std::runtime_error("Unknown Kind");
    }

    state->op(Opcode::kSetLocalObject);
    state->emitInt<uint16_t>(*statementNode.localValueIndex);
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

vector<uint8_t> emit(const ProcedureNode& procedureNode, int numLocalValues, int numLocalObjects) {
#ifdef DUMP_ASM
    std::cerr << "--start of emit--";
#endif
    ProcedureState state;
    state.op(Opcode::kInitLocals);
    state.emitInt<uint16_t>(numLocalValues);
    state.emitInt<uint16_t>(numLocalObjects);
    emitBody(*procedureNode.body, &state);
    state.op(Opcode::kReturn);
#ifdef DUMP_ASM
    std::cerr << std::endl << "--end of emit--" << std::endl;
#endif
    return state.bytecode;
}

}  // namespace compiler
