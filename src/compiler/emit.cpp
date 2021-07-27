// uncomment to dump the generated assembly to std::cerr
// #define DUMP_ASM

#include "emit.h"
#include "CompilerException.h"
#include "util/decimal.h"
#include "vm/Opcode.h"
#include "vm/systemCall.h"

using std::array;
using std::vector;
using vm::Opcode;
using vm::SystemCall;

namespace compiler {

class JumpFixup {
   public:
    size_t bytecodeOffset{};
    int labelId{};
};

typedef JumpFixup Label;

class ProcedureState {
   public:
    vector<uint8_t> bytecode;
    vector<JumpFixup> jumpFixups;  // fixups for jumps to labels that have not been emitted yet
    vector<Label> labels;          // labels that have already been emitted
    int nextLabelId = 1;

    int labelId() { return nextLabelId++; }

    void pushImmediateUtf8(const std::string& str) {
        op(Opcode::kPushImmediateUtf8, false);
        emitInt<uint32_t>(str.size(), false);
        for (auto ch : str) {
            emitInt<uint8_t>(ch, false);
        }
#ifdef DUMP_ASM
        std::cerr << std::endl << NAMEOF_ENUM(Opcode::kPushImmediateUtf8) << " \"" << str << "\"";
#endif
    }

    void label(int labelId) {
        for (const auto& fixup : jumpFixups) {
            if (fixup.labelId == labelId) {
                auto target = static_cast<uint32_t>(bytecode.size());
                memcpy(&bytecode.at(fixup.bytecodeOffset), &target, sizeof(uint32_t));
            }
        }
        jumpFixups.erase(
            std::remove_if(
                jumpFixups.begin(), jumpFixups.end(),
                [labelId](const JumpFixup& fixup) { return fixup.labelId == labelId; }),
            jumpFixups.end());
        labels.push_back(Label{ bytecode.size(), labelId });
    }

    void pushImmediateInt64(int64_t value) {
        op(Opcode::kPushImmediateInt64);
        emitInt<int64_t>(value);
    }

    void pushImmediateDec128(const decimal::Decimal& value) {
        op(Opcode::kPushImmediateDec128);
        auto triple = value.as_uint128_triple();
        emitInt<uint8_t>(triple.tag, false);
        emitInt<uint8_t>(triple.sign, false);
        emitInt<uint64_t>(triple.hi, false);
        emitInt<uint64_t>(triple.lo, false);
        emitInt<int64_t>(triple.exp, false);
#ifdef DUMP_ASM
        std::cerr << " " << util::decimalToString(value);
#endif
    }

    void initLocals(uint16_t numVals, uint16_t numObjs) {
        op(Opcode::kInitLocals);
        emitInt<uint16_t>(numVals);
        emitInt<uint16_t>(numObjs);
    }

    void pushLocalValue(uint16_t index) {
        op(Opcode::kPushLocalValue);
        emitInt<uint16_t>(index);
    }

    void setLocalValue(uint16_t index) {
        op(Opcode::kSetLocalValue);
        emitInt<uint16_t>(index);
    }

    void pushLocalObject(uint16_t index) {
        op(Opcode::kPushLocalObject);
        emitInt<uint16_t>(index);
    }

    void setLocalObject(uint16_t index) {
        op(Opcode::kSetLocalObject);
        emitInt<uint16_t>(index);
    }

    void pushArgumentValue(uint8_t index) {
        op(Opcode::kPushArgumentValue);
        emitInt<uint8_t>(index);
    }

    void setArgumentValue(uint8_t index) {
        op(Opcode::kSetArgumentValue);
        emitInt<uint8_t>(index);
    }

    void pushArgumentObject(uint8_t index) {
        op(Opcode::kPushArgumentObject);
        emitInt<uint8_t>(index);
    }

    void setArgumentObject(uint8_t index) {
        op(Opcode::kSetArgumentObject);
        emitInt<uint8_t>(index);
    }

    void jump(int labelId) {
        op(Opcode::kJump);
        emitJumpTarget(labelId);
    }

    void branchIfTrue(int labelId) {
        op(Opcode::kBranchIfTrue);
        emitJumpTarget(labelId);
    }

    void branchIfFalse(int labelId) {
        op(Opcode::kBranchIfFalse);
        emitJumpTarget(labelId);
    }

    void call(Opcode opcode, uint32_t procedureIndex, uint8_t numVals, uint8_t numObjs) {
        op(opcode, false);
        emitInt<uint32_t>(procedureIndex, false);
        emitInt<uint8_t>(numVals, false);
        emitInt<uint8_t>(numObjs, false);
#ifdef DUMP_ASM
        std::cerr << std::endl
                  << NAMEOF_ENUM(opcode) << " " << procedureIndex << " " << static_cast<int>(numVals) << " "
                  << static_cast<int>(numObjs);
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

    void returnVoid() { op(Opcode::kReturn); }

    void returnValue() { op(Opcode::kReturnValue); }

    void returnObject() { op(Opcode::kReturnObject); }

    void recordNew(uint16_t numVals, uint16_t numObjs) {
        op(Opcode::kRecordNew);
        emitInt<uint16_t>(numVals);
        emitInt<uint16_t>(numObjs);
    }

    void valueListNew(uint16_t numVals) {
        op(Opcode::kValueListNew);
        emitInt<uint16_t>(numVals);
    }

    void objectListNew(uint16_t numObjs) {
        op(Opcode::kObjectListNew);
        emitInt<uint16_t>(numObjs);
    }

   private:
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

    void emitJumpTarget(int labelId) {
        for (auto& label : labels) {
            if (label.labelId == labelId) {
                emitInt<uint32_t>(label.bytecodeOffset, false);
                return;
            }
        }
        jumpFixups.push_back(JumpFixup{ bytecode.size(), labelId });
        emitInt<uint32_t>(0);
    }
};

static void emitExpression(const ExpressionNode& expressionNode, ProcedureState* state);
static void emitBody(const BodyNode& bodyNode, ProcedureState* state);
static void emitStatement(const StatementNode& statementNode, ProcedureState* state);

static void emitBinaryExpression(const BinaryExpressionNode& expressionNode, ProcedureState* state) {
    assert(expressionNode.leftOperand->evaluatedType);
    auto lhsType = expressionNode.leftOperand->evaluatedType;
    emitExpression(*expressionNode.leftOperand, state);
    for (const auto& binarySuffix : expressionNode.binarySuffixes) {
        if (binarySuffix->leftOperandConvertedType != nullptr) {
            throw std::runtime_error("not impl");
            lhsType = binarySuffix->leftOperandConvertedType;
        }
        auto rhsType = binarySuffix->rightOperand->evaluatedType;
        emitExpression(*binarySuffix->rightOperand, state);
        if (binarySuffix->rightOperandConvertedType != nullptr) {
            throw std::runtime_error("not impl");
            rhsType = binarySuffix->rightOperandConvertedType;
        }
        auto suffixResultType = binarySuffix->evaluatedType;
        if (lhsType->kind == Kind::kNumber && rhsType->kind == Kind::kNumber) {
            SystemCall systemCall{};
            switch (binarySuffix->binaryOperator) {
                case BinaryOperator::kEquals:
                    systemCall = SystemCall::kNumberEquals;
                    break;
                case BinaryOperator::kNotEquals:
                    systemCall = SystemCall::kNumberNotEquals;
                    break;
                case BinaryOperator::kLessThan:
                    systemCall = SystemCall::kNumberLessThan;
                    break;
                case BinaryOperator::kLessThanEquals:
                    systemCall = SystemCall::kNumberLessThanEquals;
                    break;
                case BinaryOperator::kGreaterThan:
                    systemCall = SystemCall::kNumberGreaterThan;
                    break;
                case BinaryOperator::kGreaterThanEquals:
                    systemCall = SystemCall::kNumberGreaterThanEquals;
                    break;
                case BinaryOperator::kAdd:
                    systemCall = SystemCall::kNumberAdd;
                    break;
                case BinaryOperator::kSubtract:
                    systemCall = SystemCall::kNumberSubtract;
                    break;
                case BinaryOperator::kMultiply:
                    systemCall = SystemCall::kNumberMultiply;
                    break;
                case BinaryOperator::kDivide:
                    systemCall = SystemCall::kNumberDivide;
                    break;
                case BinaryOperator::kModulus:
                    systemCall = SystemCall::kNumberModulus;
                    break;
                default:
                    throw std::runtime_error("not impl");
            }
            state->syscall(Opcode::kSystemCallV, systemCall, 2, 0);
        } else {
            throw std::runtime_error("not impl");
        }
        lhsType = suffixResultType;
    }
}

static void emitSymbolReference(const Node& declarationNode, ProcedureState* state, bool set = false) {
    // local variable
    if (declarationNode.localValueIndex.has_value()) {
        if (set) {
            state->setLocalValue(*declarationNode.localValueIndex);
        } else {
            state->pushLocalValue(*declarationNode.localValueIndex);
        }
        return;
    }
    if (declarationNode.localObjectIndex.has_value()) {
        if (set) {
            state->setLocalObject(*declarationNode.localObjectIndex);
        } else {
            state->pushLocalObject(*declarationNode.localObjectIndex);
        }
        return;
    }

    // procedure argument
    const auto* parameterNode = dynamic_cast<const ParameterNode*>(&declarationNode);
    if (parameterNode != nullptr) {
        if (parameterNode->argumentValueIndex.has_value()) {
            if (set) {
                state->setArgumentValue(*parameterNode->argumentValueIndex);
            } else {
                state->pushArgumentValue(*parameterNode->argumentValueIndex);
            }
            return;
        }
        if (parameterNode->argumentObjectIndex.has_value()) {
            if (set) {
                state->setArgumentObject(*parameterNode->argumentObjectIndex);
            } else {
                state->pushArgumentObject(*parameterNode->argumentObjectIndex);
            }
            return;
        }
    }

    // function call with zero arguments
    const auto* procedureNode = dynamic_cast<const ProcedureNode*>(&declarationNode);
    if (procedureNode != nullptr) {
        assert(!set);
        assert(procedureNode->returnType != nullptr);
        auto returnsValue = procedureNode->returnType->isValueType();
        if (declarationNode.procedureIndex.has_value()) {
            state->call(returnsValue ? Opcode::kCallV : Opcode::kCallO, *declarationNode.procedureIndex, 0, 0);
        } else if (declarationNode.systemCall.has_value()) {
            state->syscall(
                returnsValue ? Opcode::kSystemCallV : Opcode::kSystemCallO, *declarationNode.systemCall, 0, 0);
        }
        return;
    }

    throw std::runtime_error("not impl");
}

static void emitCallExpression(const CallExpressionNode& expressionNode, ProcedureState* state) {
    assert(expressionNode.evaluatedType != nullptr);
    auto returnsValue = expressionNode.evaluatedType->isValueType();
    auto numValueArgs = 0;
    auto numObjectArgs = 0;
    for (const auto& arg : expressionNode.arguments) {
        assert(arg->evaluatedType != nullptr);
        arg->evaluatedType->isValueType() ? numValueArgs++ : numObjectArgs++;
        emitExpression(*arg, state);
    }
    if (expressionNode.boundSymbolDeclaration != nullptr &&
        dynamic_cast<const ProcedureNode*>(expressionNode.boundSymbolDeclaration) == nullptr) {
        // this is a list or map index
        assert(expressionNode.boundSymbolDeclaration->evaluatedType != nullptr);
        emitSymbolReference(*expressionNode.boundSymbolDeclaration, state);
        auto* declType = expressionNode.boundSymbolDeclaration->getSymbolDeclarationType().get();
        assert(declType);
        if (declType->kind == Kind::kList) {
            auto& itemType = *declType->listItemType;
            if (itemType.isValueType()) {
                state->syscall(Opcode::kSystemCallV, SystemCall::kValueListGet, 1, 1);
            } else {
                state->syscall(Opcode::kSystemCallO, SystemCall::kObjectListGet, 1, 1);
            }
        } else if (declType->kind == Kind::kMap) {
            throw std::runtime_error("not impl");
        } else {
            throw CompilerException("Internal error. Unexpected type.", expressionNode.boundSymbolDeclaration->token);
        }
    } else if (expressionNode.procedureIndex.has_value()) {
        state->call(
            returnsValue ? Opcode::kCallV : Opcode::kCallO, *expressionNode.procedureIndex, numValueArgs,
            numObjectArgs);
    } else if (expressionNode.systemCall.has_value()) {
        state->syscall(
            returnsValue ? Opcode::kSystemCallV : Opcode::kSystemCallO, *expressionNode.systemCall, numValueArgs,
            numObjectArgs);
    } else {
        throw std::runtime_error("not impl");
    }
}

static void emitLiteralArrayExpression(const LiteralArrayExpressionNode& expressionNode, ProcedureState* state) {
    assert(!expressionNode.elements.empty());
    assert(expressionNode.elements.at(0)->evaluatedType != nullptr);
    auto& elementType = *expressionNode.elements.at(0)->evaluatedType;
    auto isValueList = elementType.isValueType();
    for (const auto& element : expressionNode.elements) {
        emitExpression(*element, state);
    }
    auto count = expressionNode.elements.size();
    if (count > std::numeric_limits<uint16_t>::max()) {
        throw CompilerException("Too many elements in this literal list.", expressionNode.token);
    }
    if (isValueList) {
        state->valueListNew(static_cast<uint16_t>(count));
    } else {
        state->objectListNew(static_cast<uint16_t>(count));
    }
}

static void emitLiteralBooleanExpression(const LiteralBooleanExpressionNode& expressionNode, ProcedureState* state) {
    state->pushImmediateInt64(expressionNode.value ? 1 : 0);
}

static void emitLiteralNumberExpression(const LiteralNumberExpressionNode& expressionNode, ProcedureState* state) {
    state->pushImmediateDec128(expressionNode.value);
}

static void emitLiteralRecordExpression(
    const LiteralRecordExpressionNode& /*expressionNode*/,
    ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitLiteralStringExpression(const LiteralStringExpressionNode& expressionNode, ProcedureState* state) {
    state->pushImmediateUtf8(expressionNode.value);
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

static void emitConvertExpression(const ConvertExpressionNode& /*expressionNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitDottedExpression(const DottedExpressionNode& /*expressionNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitNotExpression(const NotExpressionNode& /*expressionNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitSymbolReferenceExpression(const SymbolReferenceExpressionNode& expressionNode, ProcedureState* state) {
    const auto* decl = expressionNode.boundSymbolDeclaration;
    assert(decl != nullptr);
    emitSymbolReference(*decl, state);
}

/*static*/ void emitExpression(const ExpressionNode& expressionNode, ProcedureState* state) {
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
    if (!statementNode.suffixes.empty()) {
        throw std::runtime_error("not impl");
    }

    emitExpression(*statementNode.value, state);
    assert(statementNode.symbolReference->boundSymbolDeclaration != nullptr);
    emitSymbolReference(*statementNode.symbolReference->boundSymbolDeclaration, state, true);
}

static void emitCallStatement(const CallStatementNode& statementNode, ProcedureState* state) {
    assert(statementNode.procedureIndex.has_value());
    auto numValueArgs = 0;
    auto numObjectArgs = 0;
    int maxArgs = std::numeric_limits<uint8_t>::max();
    for (const auto& arg : statementNode.arguments) {
        assert(arg->evaluatedType != nullptr);
        arg->evaluatedType->isValueType() ? numValueArgs++ : numObjectArgs++;
        emitExpression(*arg, state);
        if (numValueArgs > maxArgs || numObjectArgs > maxArgs) {
            throw CompilerException("Too many arguments in call.", statementNode.token);
        }
    }
    state->call(
        Opcode::kCall, *statementNode.procedureIndex, static_cast<uint8_t>(numValueArgs),
        static_cast<uint8_t>(numObjectArgs));
}

static void emitConstStatement(const ConstStatementNode& /*statementNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitContinueStatement(const ContinueStatementNode& /*statementNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitDimListStatement(const DimListStatementNode& /*statementNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitDimMapStatement(const DimMapStatementNode& /*statementNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitDimStatement(const DimStatementNode& statementNode, ProcedureState* state) {
    assert(statementNode.evaluatedType != nullptr);
    const auto& type = *statementNode.evaluatedType;

    // if an initial value is provided, then use that
    if (statementNode.value != nullptr) {
        emitExpression(*statementNode.value, state);
        if (statementNode.localValueIndex.has_value()) {
            state->setLocalValue(*statementNode.localValueIndex);
        } else if (statementNode.localObjectIndex.has_value()) {
            state->setLocalObject(*statementNode.localObjectIndex);
        } else {
            throw std::runtime_error("Internal error. No local variable index found!");
        }
        return;
    }

    // initialize all value types to 0.
    if (type.isValueType()) {
        assert(statementNode.localValueIndex >= 0);
        state->pushImmediateInt64(0);
        state->setLocalValue(*statementNode.localValueIndex);
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
            state->pushImmediateInt64(0);
            state->pushImmediateInt64(0);
            state->recordNew(2, 0);
            break;

        case Kind::kList:
            if (type.listItemType->isValueType()) {
                state->valueListNew(0);
            } else {
                state->objectListNew(0);
            }
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
            state->pushImmediateUtf8("");
            break;

        case Kind::kTimeZone:
            state->pushImmediateUtf8("UTC");
            state->syscall(Opcode::kSystemCallO, SystemCall::kTimeZoneFromName, 0, 1);
            break;

        default:
            throw std::runtime_error("Unknown Kind");
    }

    state->setLocalObject(*statementNode.localValueIndex);
}

static void emitDoStatement(const DoStatementNode& /*statementNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitExitStatement(const ExitStatementNode& /*statementNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitForEachStatement(const ForEachStatementNode& /*statementNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitForStatement(const ForStatementNode& statementNode, ProcedureState* state) {
    // we need two local temp variables
    assert(statementNode.getTempLocalValueCount() == 2);
    assert(statementNode.tempLocalValueIndex.has_value());
    auto toLocalValueIndex = *statementNode.tempLocalValueIndex;
    auto stepLocalValueIndex = toLocalValueIndex + 1;

    // plus the loop counter variable which is user-visible
    assert(statementNode.localValueIndex.has_value());
    auto counterLocalValueIndex = *statementNode.localValueIndex;

    // evaluate the fromValue and store in the counter variable
    emitExpression(*statementNode.fromValue, state);
    state->setLocalValue(counterLocalValueIndex);

    // evaluate the toValue and store in a temp variable
    emitExpression(*statementNode.toValue, state);
    state->setLocalValue(toLocalValueIndex);

    // evaluate the step value and store in a temp variable
    if (statementNode.step == nullptr) {
        state->pushImmediateInt64(1);
    } else {
        emitExpression(*statementNode.step, state);
    }
    state->setLocalValue(stepLocalValueIndex);

    // loop begins here
    auto topLabel = state->labelId();
    auto endLabel = state->labelId();
    state->label(topLabel);

    // if the loop variable is beyond toValue, then jump to endLabel
    state->pushLocalValue(counterLocalValueIndex);
    state->pushLocalValue(toLocalValueIndex);
    state->pushLocalValue(stepLocalValueIndex);
    state->syscall(Opcode::kSystemCallV, SystemCall::kCounterIsPastLimit, 3, 0);
    state->branchIfTrue(endLabel);

    // loop body
    emitBody(*statementNode.body, state);

    // increment the counter by the step
    state->pushLocalValue(counterLocalValueIndex);
    state->pushLocalValue(stepLocalValueIndex);
    state->syscall(Opcode::kSystemCallV, SystemCall::kNumberAdd, 2, 0);
    state->setLocalValue(counterLocalValueIndex);

    // jump to topLabel
    state->jump(topLabel);

    // we will jump here when the loop is done
    state->label(endLabel);
}

static void emitGroupStatement(const GroupStatementNode& /*statementNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitIfStatement(const IfStatementNode& statementNode, ProcedureState* state) {
    emitExpression(*statementNode.condition, state);
    auto elseIfsLabel = state->labelId();
    auto endIfLabel = state->labelId();
    state->branchIfFalse(elseIfsLabel);
    emitBody(*statementNode.body, state);
    state->jump(endIfLabel);
    state->label(elseIfsLabel);
    for (const auto& elseIf : statementNode.elseIfs) {
        auto endElseIfLabel = state->labelId();
        emitExpression(*elseIf->condition, state);
        state->branchIfFalse(endElseIfLabel);
        emitBody(*elseIf->body, state);
        state->jump(endIfLabel);
        state->label(endElseIfLabel);
    }
    if (statementNode.elseBody != nullptr) {
        emitBody(*statementNode.elseBody, state);
    }
    state->label(endIfLabel);
}

static void emitJoinStatement(const JoinStatementNode& /*statementNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitRethrowStatement(const RethrowStatementNode& /*statementNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitReturnStatement(const ReturnStatementNode& statementNode, ProcedureState* state) {
    assert(statementNode.expression->evaluatedType != nullptr);
    emitExpression(*statementNode.expression, state);
    auto isValue = statementNode.expression->evaluatedType->isValueType();
    if (isValue) {
        state->returnValue();
    } else {
        state->returnObject();
    }
}

static void emitSelectCaseStatement(const SelectCaseStatementNode& /*statementNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitSelectStatement(const SelectStatementNode& /*statementNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitThrowStatement(const ThrowStatementNode& /*statementNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitTryStatement(const TryStatementNode& /*statementNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitWhileStatement(const WhileStatementNode& statementNode, ProcedureState* state) {
    // loop begins here
    auto topLabel = state->labelId();
    auto endLabel = state->labelId();
    state->label(topLabel);

    // evaluate the condition, if it's false then jump to endLabel
    emitExpression(*statementNode.condition, state);
    state->branchIfFalse(endLabel);

    // loop body
    emitBody(*statementNode.body, state);

    // jump to topLabel
    state->jump(topLabel);

    // we will jump here when the loop is done
    state->label(endLabel);
}

static void emitPrintStatement(const PrintStatementNode& statementNode, ProcedureState* state) {
    for (const auto& expressionNode : statementNode.expressions) {
        assert(expressionNode->evaluatedType != nullptr);
        emitExpression(*expressionNode, state);
        switch (expressionNode->evaluatedType->kind) {
            case Kind::kBoolean: {
                auto printFalseLabel = state->labelId();
                auto endPrintLabel = state->labelId();
                state->branchIfFalse(printFalseLabel);
                state->pushImmediateUtf8("true");
                state->jump(endPrintLabel);
                state->label(printFalseLabel);
                state->pushImmediateUtf8("false");
                state->label(endPrintLabel);
                state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
                break;
            }
            case Kind::kNumber:
                state->syscall(Opcode::kSystemCallO, SystemCall::kNumberToString, 1, 0);
                state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
                break;
            case Kind::kDate:
                state->syscall(Opcode::kSystemCallO, SystemCall::kDateToString, 1, 0);
                state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
                break;
            case Kind::kDateTime:
                state->syscall(Opcode::kSystemCallO, SystemCall::kDateTimeToString, 1, 0);
                state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
                break;
            case Kind::kDateTimeOffset:
                state->syscall(Opcode::kSystemCallO, SystemCall::kDateTimeOffsetToString, 0, 1);
                state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
                break;
            case Kind::kTimeSpan:
                state->syscall(Opcode::kSystemCallO, SystemCall::kTimeSpanToString, 1, 0);
                state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
                break;
            case Kind::kTimeZone:
                state->syscall(Opcode::kSystemCallO, SystemCall::kTimeZoneToString, 0, 1);
                state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
                break;
            case Kind::kString:
                state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
                break;
            case Kind::kList:
                throw std::runtime_error("not impl");
            case Kind::kMap:
                throw std::runtime_error("not impl");
            case Kind::kRecord:
                throw std::runtime_error("not impl");
            case Kind::kOptional:
                throw std::runtime_error("not impl");
            default:
                throw CompilerException("Unknown Kind", expressionNode->token);
        }
    }

    if (!statementNode.trailingSemicolon) {
        state->pushImmediateUtf8("\n");
        state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
    }

    state->syscall(Opcode::kSystemCall, SystemCall::kFlushConsoleOutput, 0, 0);
}

static void emitInputStatement(const InputStatementNode& statementNode, ProcedureState* state) {
    assert(statementNode.target->evaluatedType != nullptr);
    auto* targetSymbolReference = dynamic_cast<SymbolReferenceExpressionNode*>(statementNode.target.get());
    if (targetSymbolReference == nullptr) {
        throw CompilerException(
            "The target of an \"input\" statement must be the name of a variable, not a more complicated expression.",
            statementNode.target->token);
    }
    state->syscall(Opcode::kSystemCallO, SystemCall::kInputString, 0, 0);
    assert(targetSymbolReference->boundSymbolDeclaration != nullptr);
    emitSymbolReference(*targetSymbolReference->boundSymbolDeclaration, state, true);
}

/*static*/ void emitStatement(const StatementNode& statementNode, ProcedureState* state) {
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

/*static*/ void emitBody(const BodyNode& bodyNode, ProcedureState* state) {
    for (const auto& statement : bodyNode.statements) {
        emitStatement(*statement, state);
    }
}

vector<uint8_t> emit(const ProcedureNode& procedureNode, int numLocalValues, int numLocalObjects) {
#ifdef DUMP_ASM
    std::cerr << "--start of emit--";
#endif
    ProcedureState state;
    int maxLocals = std::numeric_limits<uint16_t>::max();
    if (numLocalValues > maxLocals || numLocalObjects > maxLocals) {
        throw CompilerException("Too many local variables.", procedureNode.token);
    }
    state.initLocals(static_cast<uint16_t>(numLocalValues), static_cast<uint16_t>(numLocalObjects));
    emitBody(*procedureNode.body, &state);
    state.returnVoid();
#ifdef DUMP_ASM
    std::cerr << std::endl << "--end of emit--" << std::endl;
#endif
    return state.bytecode;
}

}  // namespace compiler
