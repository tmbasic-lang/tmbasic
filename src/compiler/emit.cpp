// uncomment to dump the generated assembly to std::cerr
// #define DUMP_ASM

#include "emit.h"
#include "CompilerException.h"
#include "util/decimal.h"
#include "vm/Opcode.h"
#include "vm/systemCall.h"

using std::array;
using std::stack;
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

class LoopFrame {
   public:
    LoopType loopType;
    int continueLabelId;
    int exitLabelId;
    LoopFrame(LoopType loopType, int continueLabelId, int exitLabelId)
        : loopType(loopType), continueLabelId(continueLabelId), exitLabelId(exitLabelId) {}
};

class ProcedureState {
   public:
    vector<uint8_t> bytecode;
    vector<JumpFixup> jumpFixups;  // fixups for jumps to labels that have not been emitted yet
    vector<Label> labels;          // labels that have already been emitted
    int nextLabelId = 1;
    stack<int> catchLabelIds;
    stack<LoopFrame> loopFrames;

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
#ifdef DUMP_ASM
        std::cerr << std::endl << "label " << labelId;
#endif
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

    void popValue() { op(Opcode::kPopValue); }
    void popObject() { op(Opcode::kPopObject); }
    void duplicateValue() { op(Opcode::kDuplicateValue); }
    void duplicateObject() { op(Opcode::kDuplicateObject); }
    void swapValues() { op(Opcode::kSwapValues); }
    void swapObjects() { op(Opcode::kSwapObjects); }

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

    void clearLocalObject(uint16_t index) {
        op(Opcode::kClearLocalObject);
        emitInt<uint16_t>(index);
    }

    void pushGlobalValue(uint16_t index) {
        op(Opcode::kPushGlobalValue);
        emitInt<uint16_t>(index);
    }

    void setGlobalValue(uint16_t index) {
        op(Opcode::kSetGlobalValue);
        emitInt<uint16_t>(index);
    }

    void pushGlobalObject(uint16_t index) {
        op(Opcode::kPushGlobalObject);
        emitInt<uint16_t>(index);
    }

    void setGlobalObject(uint16_t index) {
        op(Opcode::kSetGlobalObject);
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

    void returnOrBranchIfError() {
        if (catchLabelIds.empty()) {
            returnIfError();
        } else {
            branchIfError(catchLabelIds.top());
        }
    }

    void call(Opcode opcode, uint32_t procedureIndex, uint8_t numVals, uint8_t numObjs) {
        op(opcode, false);
        emitInt<uint32_t>(procedureIndex, false);
        emitInt<uint8_t>(numVals, false);
        emitInt<uint8_t>(numObjs, false);

        returnOrBranchIfError();

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

        if (catchLabelIds.empty()) {
            returnIfError();
        } else {
            branchIfError(catchLabelIds.top());
        }
    }

    void returnVoid() { op(Opcode::kReturn); }
    void returnValue() { op(Opcode::kReturnValue); }
    void returnObject() { op(Opcode::kReturnObject); }
    void setError() { op(Opcode::kSetError); }
    void clearError() { op(Opcode::kClearError); }
    void bubbleError() { op(Opcode::kBubbleError); }
    void returnIfError() { op(Opcode::kReturnIfError); }

    void branchIfError(int labelId) {
        op(Opcode::kBranchIfError);
        emitJumpTarget(labelId);
    }

    void recordNew(uint16_t numVals, uint16_t numObjs) {
        op(Opcode::kRecordNew);
        emitInt<uint16_t>(numVals);
        emitInt<uint16_t>(numObjs);
    }

    void recordGetValue(uint16_t index) {
        op(Opcode::kRecordGetValue);
        emitInt<uint16_t>(index);
    }

    void recordGetObject(uint16_t index) {
        op(Opcode::kRecordGetObject);
        emitInt<uint16_t>(index);
    }

    void recordSetValue(uint16_t index) {
        op(Opcode::kRecordSetValue);
        emitInt<uint16_t>(index);
    }

    void recordSetObject(uint16_t index) {
        op(Opcode::kRecordSetObject);
        emitInt<uint16_t>(index);
    }

    void valueListNew(uint16_t numVals) {
        op(Opcode::kValueListNew);
        emitInt<uint16_t>(numVals);
    }

    void objectListNew(uint16_t numObjs) {
        op(Opcode::kObjectListNew);
        emitInt<uint16_t>(numObjs);
    }

    void dottedExpressionSetValue(uint8_t numSuffixes, uint8_t numKeyValuesOnStack, uint8_t numKeyObjectsOnStack) {
        op(Opcode::kDottedExpressionSetValue);
        emitInt<uint8_t>(numSuffixes);
        emitInt<uint8_t>(numKeyValuesOnStack);
        emitInt<uint8_t>(numKeyObjectsOnStack);
    }

    void dottedExpressionSetObject(uint8_t numSuffixes, uint8_t numKeyValuesOnStack, uint8_t numKeyObjectsOnStack) {
        op(Opcode::kDottedExpressionSetObject);
        emitInt<uint8_t>(numSuffixes);
        emitInt<uint8_t>(numKeyValuesOnStack);
        emitInt<uint8_t>(numKeyObjectsOnStack);
    }

    void dottedExpressionDottedSuffix(bool isValueField, uint16_t fieldIndex) {
        emitInt<uint8_t>(isValueField ? 1 : 2);
        emitInt<uint16_t>(fieldIndex);
    }

    void dottedExpressionValueKeySuffix(bool isValueElement) { emitInt<uint8_t>(isValueElement ? 3 : 4); }

    void dottedExpressionObjectKeySuffix(bool isValueElement) { emitInt<uint8_t>(isValueElement ? 5 : 6); }

    void objectToObjectMapTryGet() { op(Opcode::kObjectToObjectMapTryGet); }
    void objectToValueMapTryGet() { op(Opcode::kObjectToValueMapTryGet); }
    void valueToObjectMapTryGet() { op(Opcode::kValueToObjectMapTryGet); }
    void valueToValueMapTryGet() { op(Opcode::kValueToValueMapTryGet); }
    void setErrorMapKeyNotFound() { op(Opcode::kSetErrorMapKeyNotFound); }

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
        emitInt<uint32_t>(0, false);
#ifdef DUMP_ASM
        std::cerr << " " << labelId;
#endif
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
        // handle boolean AND/OR short circuiting
        auto isOr = binarySuffix->binaryOperator == BinaryOperator::kOr;
        auto isAnd = binarySuffix->binaryOperator == BinaryOperator::kAnd;
        auto endOfBinarySuffixLabel = state->labelId();
        if (lhsType->kind == Kind::kBoolean && (isOr || isAnd)) {
            if (isOr) {
                // if LHS=true, then skip evaluating RHS and keep the true value
                state->duplicateValue();
                state->branchIfTrue(endOfBinarySuffixLabel);
            } else if (isAnd) {
                // if LHS=false, then skip evaluating RHS and keep the false value
                state->duplicateValue();
                state->branchIfFalse(endOfBinarySuffixLabel);
            }
        }

        auto rhsType = binarySuffix->rightOperand->evaluatedType;
        emitExpression(*binarySuffix->rightOperand, state);
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
                case BinaryOperator::kPower:
                    systemCall = SystemCall::kPow;
                    break;
                default:
                    throw CompilerException(
                        CompilerErrorCode::kInternal, "Internal error. Unimplemented binary operator.",
                        expressionNode.token);
            }
            state->syscall(Opcode::kSystemCallV, systemCall, 2, 0);
        } else if (lhsType->kind == Kind::kString && rhsType->kind == Kind::kString) {
            switch (binarySuffix->binaryOperator) {
                case BinaryOperator::kAdd:
                    state->syscall(Opcode::kSystemCallO, SystemCall::kStringConcat, 0, 2);
                    break;
                case BinaryOperator::kEquals:
                    state->syscall(Opcode::kSystemCallV, SystemCall::kStringEquals, 0, 2);
                    break;
                default:
                    throw CompilerException(
                        CompilerErrorCode::kInternal, "Internal error. Unimplemented binary operator.",
                        expressionNode.token);
            }
        } else if (lhsType->kind == Kind::kBoolean && rhsType->kind == Kind::kBoolean) {
            SystemCall systemCall{};
            switch (binarySuffix->binaryOperator) {
                case BinaryOperator::kEquals:
                    systemCall = SystemCall::kNumberEquals;
                    break;
                case BinaryOperator::kNotEquals:
                    systemCall = SystemCall::kNumberNotEquals;
                    break;
                case BinaryOperator::kAnd:
                    systemCall = SystemCall::kBooleanAnd;
                    break;
                case BinaryOperator::kOr:
                    systemCall = SystemCall::kBooleanOr;
                    break;
                default:
                    throw CompilerException(
                        CompilerErrorCode::kInternal, "Internal error. Unimplemented binary operator.",
                        expressionNode.token);
            }
            state->syscall(Opcode::kSystemCallV, systemCall, 2, 0);
        } else if (lhsType->kind == Kind::kList && rhsType->kind == Kind::kList) {
            // Careful! Just because these are both lists doesn't mean they are the same kind of list.

            // Example 1: Here we CONCAT two lists together.
            //   dim a as List of Number
            //   dim b as List of Number
            //   a = a + b

            // Example 2: Here we APPEND a single item to a list of lists.
            //   dim a as List of List of Number
            //   dim b as List of Number
            //   a = a + b

            // We need to handle both situations here.

            auto lhsEqualsRhs = lhsType->equals(*rhsType);                      // concat
            auto rhsIsItemTypeOfLhs = lhsType->listItemType->equals(*rhsType);  // append

            // Must be exactly one of these two situations. Type checking should have caught this already.
            if (lhsEqualsRhs == rhsIsItemTypeOfLhs) {
                throw CompilerException(
                    CompilerErrorCode::kInternal, "Internal error. Unexpected operand types to + operator.",
                    expressionNode.token);
            }

            if (lhsEqualsRhs) {
                // Concat (example 1).
                switch (binarySuffix->binaryOperator) {
                    case BinaryOperator::kAdd:
                        if (lhsType->listItemType->isValueType()) {
                            state->syscall(Opcode::kSystemCallO, SystemCall::kValueListConcat, 0, 2);
                        } else {
                            state->syscall(Opcode::kSystemCallO, SystemCall::kObjectListConcat, 0, 2);
                        }
                        break;
                    default:
                        throw CompilerException(
                            CompilerErrorCode::kInternal, "Internal error. Unimplemented binary operator.",
                            expressionNode.token);
                }
            } else {
                // Append (example 2).
                switch (binarySuffix->binaryOperator) {
                    case BinaryOperator::kAdd:
                        if (lhsType->listItemType->isValueType()) {
                            state->syscall(Opcode::kSystemCallO, SystemCall::kValueListAdd, 1, 1);
                        } else {
                            state->syscall(Opcode::kSystemCallO, SystemCall::kObjectListAdd, 0, 2);
                        }
                        break;
                    default:
                        throw CompilerException(
                            CompilerErrorCode::kInternal, "Internal error. Unimplemented binary operator.",
                            expressionNode.token);
                }
            }
        } else if (lhsType->kind == Kind::kList && rhsType->kind != Kind::kList) {
            assert(binarySuffix->binaryOperator == BinaryOperator::kAdd);
            if (lhsType->listItemType->isValueType()) {
                state->syscall(Opcode::kSystemCallO, SystemCall::kValueListAdd, 1, 1);
            } else {
                state->syscall(Opcode::kSystemCallO, SystemCall::kObjectListAdd, 0, 2);
            }
        } else {
            throw std::runtime_error("not impl");
        }
        lhsType = suffixResultType;

        state->label(endOfBinarySuffixLabel);
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
    if (declarationNode.globalValueIndex.has_value()) {
        if (set) {
            state->setGlobalValue(*declarationNode.globalValueIndex);
        } else {
            state->pushGlobalValue(*declarationNode.globalValueIndex);
        }
        return;
    }
    if (declarationNode.globalObjectIndex.has_value()) {
        if (set) {
            state->setGlobalObject(*declarationNode.globalObjectIndex);
        } else {
            state->pushGlobalObject(*declarationNode.globalObjectIndex);
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

    // constant
    const auto* constStatementNode = dynamic_cast<const ConstStatementNode*>(&declarationNode);
    if (constStatementNode != nullptr) {
        assert(!set);
        switch (constStatementNode->evaluatedType->kind) {
            case Kind::kNumber: {
                const auto& literalNumberExpr =
                    dynamic_cast<const LiteralNumberExpressionNode&>(*constStatementNode->value);
                state->pushImmediateDec128(literalNumberExpr.value);
                break;
            }

            case Kind::kString: {
                const auto& literalStringExpr =
                    dynamic_cast<const LiteralStringExpressionNode&>(*constStatementNode->value);
                state->pushImmediateUtf8(literalStringExpr.value);
                break;
            }

            case Kind::kBoolean: {
                const auto& literalBooleanExpr =
                    dynamic_cast<const LiteralBooleanExpressionNode&>(*constStatementNode->value);
                state->pushImmediateInt64(literalBooleanExpr.value ? 1 : 0);
                break;
            }

            default:
                throw std::runtime_error("not impl");
        }
        return;
    }

    throw CompilerException(
        CompilerErrorCode::kInternal, "Internal error. Unknown declaration node type.", declarationNode.token);
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
        throw CompilerException(
            CompilerErrorCode::kTooManyLiteralListElements, "Too many elements in this literal list.",
            expressionNode.token);
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

static void emitLiteralRecordExpression(const LiteralRecordExpressionNode& expressionNode, ProcedureState* state) {
    auto numValues = 0;
    auto numObjects = 0;
    for (const auto& field : expressionNode.fields) {
        emitExpression(*field->value, state);
        if (field->value->evaluatedType->isValueType()) {
            numValues++;
        } else {
            numObjects++;
        }
    }
    state->recordNew(numValues, numObjects);
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

static void emitConvertExpression(const ConvertExpressionNode& expressionNode, ProcedureState* state) {
    // MARKER: This function concerns implicit type conversions. Search for this line to find others.

    auto& srcType = *expressionNode.value->evaluatedType;
    auto& dstType = *expressionNode.type;
    auto srcKind = expressionNode.value->evaluatedType->kind;
    auto dstKind = expressionNode.type->kind;

    emitExpression(*expressionNode.value, state);

    // Record -> Record
    if (srcKind == Kind::kRecord && dstKind == Kind::kRecord) {
        // Named and anonymous types with the same fields are indistinguishable at runtime; no conversion needed.
        return;
    }

    // Number -> String
    if (srcKind == Kind::kNumber && dstKind == Kind::kString) {
        state->syscall(Opcode::kSystemCallO, SystemCall::kNumberToString, 1, 0);
        return;
    }

    // Date -> DateTime
    if (srcKind == Kind::kDate && dstKind == Kind::kDateTime) {
        // A Date is just a DateTime where the time is zero, so we don't need to convert.
        return;
    }

    // DateTime -> Date
    if (srcKind == Kind::kDateTime && dstKind == Kind::kDate) {
        state->syscall(Opcode::kSystemCallV, SystemCall::kDateTimeToDate, 1, 0);
        return;
    }

    if (srcKind == Kind::kDateTimeOffset) {
        // DateTimeOffset -> Date
        if (dstKind == Kind::kDate) {
            state->syscall(Opcode::kSystemCallV, SystemCall::kDateTimeOffsetToDate, 1, 0);
            return;
        }

        // DateTimeOffset -> DateTime
        if (dstKind == Kind::kDateTime) {
            state->syscall(Opcode::kSystemCallV, SystemCall::kDateTimeOffsetToDateTime, 1, 0);
            return;
        }
    }

    // T -> Optional T
    if (dstKind == Kind::kOptional && dstType.optionalValueType->equals(srcType)) {
        if (srcType.isValueType()) {
            state->syscall(Opcode::kSystemCallO, SystemCall::kValueOptionalNewPresent, 1, 0);
        } else {
            state->syscall(Opcode::kSystemCallO, SystemCall::kObjectOptionalNewPresent, 0, 1);
        }
        return;
    }

    throw CompilerException(
        CompilerErrorCode::kInternal,
        "Internal error. Code generation for this implicit type conversion is not implemented.", expressionNode.token);
}

static void emitFunctionCallExpression(const FunctionCallExpressionNode& expressionNode, ProcedureState* state) {
    assert(expressionNode.procedureIndex.has_value() || expressionNode.systemCall.has_value());
    assert(expressionNode.evaluatedType != nullptr);
    auto numValueArgs = 0;
    auto numObjectArgs = 0;
    for (const auto& arg : expressionNode.args) {
        assert(arg->evaluatedType != nullptr);
        arg->evaluatedType->isValueType() ? numValueArgs++ : numObjectArgs++;
        emitExpression(*arg, state);
    }
    if (expressionNode.systemCall.has_value()) {
        auto systemCall = *expressionNode.systemCall;
        auto dualSystemCall = vm::getDualGenericSystemCall(*expressionNode.systemCall);
        if (dualSystemCall.has_value()) {
            // There is one system call when the first argument is a value and another when it is an object.
            // expressionNode.systemCall is the one for objects and *dualSystemCall is the one for values.
            // So if the first argument is a value, we have to swap over to dualSystemCall.
            assert(!expressionNode.args.empty());
            const auto& firstArg = *expressionNode.args.at(0);
            assert(firstArg.evaluatedType != nullptr);
            if (firstArg.evaluatedType->isValueType()) {
                systemCall = *dualSystemCall;
            }
        }

        state->syscall(
            expressionNode.evaluatedType->isValueType() ? Opcode::kSystemCallV : Opcode::kSystemCallO, systemCall,
            numValueArgs, numObjectArgs);
    } else {
        state->call(
            expressionNode.evaluatedType->isValueType() ? Opcode::kCallV : Opcode::kCallO,
            *expressionNode.procedureIndex, numValueArgs, numObjectArgs);
    }
}

static void emitDottedExpression(const DottedExpressionNode& expressionNode, ProcedureState* state) {
    // In fixDottedExpressionFunctionCalls() we may have transformed the suffix into part of the base, leaving no
    // suffixes left in the list here.
    if (expressionNode.dottedSuffixes.empty()) {
        emitExpression(*expressionNode.base, state);
        return;
    }

    auto* baseType = expressionNode.base->evaluatedType.get();
    emitExpression(*expressionNode.base, state);

    for (const auto& dottedSuffix : expressionNode.dottedSuffixes) {
        if (dottedSuffix->isFieldAccess()) {
            auto* parameterNode = dottedSuffix->boundParameterNode;
            assert(parameterNode != nullptr);
            baseType = parameterNode->type.get();
            assert(baseType != nullptr);
            if (parameterNode->fieldValueIndex.has_value()) {
                state->recordGetValue(*parameterNode->fieldValueIndex);
            } else if (parameterNode->fieldObjectIndex.has_value()) {
                state->recordGetObject(*parameterNode->fieldObjectIndex);
            } else {
                throw CompilerException(
                    CompilerErrorCode::kInternal,
                    "Internal error. Dotted suffix has neither fieldValueIndex nor fieldObjectIndex set.",
                    expressionNode.token);
            }
        } else if (dottedSuffix->isIndex()) {
            if (dottedSuffix->collectionIndex.size() != 1) {
                throw CompilerException(
                    CompilerErrorCode::kTooManyIndexArguments,
                    "Collection access should have exactly one index argument.", expressionNode.token);
            }
            emitExpression(*dottedSuffix->collectionIndex.at(0), state);
            if (baseType->kind == Kind::kList) {
                if (baseType->listItemType->isValueType()) {
                    state->syscall(Opcode::kSystemCallV, SystemCall::kValueListGet, 1, 1);
                } else {
                    state->syscall(Opcode::kSystemCallO, SystemCall::kObjectListGet, 1, 1);
                }
            } else if (baseType->kind == Kind::kMap) {
                assert(baseType->mapKeyType != nullptr);
                assert(baseType->mapValueType != nullptr);
                if (baseType->mapKeyType->isValueType()) {
                    if (baseType->mapValueType->isValueType()) {
                        state->valueToValueMapTryGet();
                    } else {
                        state->valueToObjectMapTryGet();
                    }
                } else {
                    if (baseType->mapValueType->isValueType()) {
                        state->objectToValueMapTryGet();
                    } else {
                        state->objectToObjectMapTryGet();
                    }
                }

                // Success boolean is on the value stack
                auto continueLabel = state->labelId();
                state->branchIfTrue(continueLabel);
                state->setErrorMapKeyNotFound();
                state->returnOrBranchIfError();
                state->label(continueLabel);
            } else {
                throw std::runtime_error("not impl");
            }
        } else {
            assert(false);
        }
    }
}

static void emitNotExpression(const NotExpressionNode& expressionNode, ProcedureState* state) {
    emitExpression(*expressionNode.operand, state);
    state->syscall(Opcode::kSystemCallV, SystemCall::kBooleanNot, 1, 0);
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
        case ExpressionType::kFunctionCall:
            emitFunctionCallExpression(dynamic_cast<const FunctionCallExpressionNode&>(expressionNode), state);
            break;
        default:
            assert(false);
            throw std::runtime_error("Unknown ExpressionType");
    }
}

static void emitAssignToExpression(
    const ExpressionNode& expressionNode,
    const ExpressionNode& rhs,
    ProcedureState* state);

static void emitAssignToDottedExpression(
    const DottedExpressionNode& expressionNode,
    const ExpressionNode& rhs,
    ProcedureState* state) {
    if (expressionNode.base->getExpressionType() != ExpressionType::kSymbolReference) {
        throw CompilerException(
            CompilerErrorCode::kInvalidAssignmentTarget,
            "The target on the left side of this assignment cannot be assigned to.", expressionNode.token);
    }

    // kDottedExpressionSetValue/Object expects the new value/object (RHS) to be pushed first.

    emitExpression(rhs, state);

    // We will push the base, then kDottedExpressionSetValue/Object will replace it with a modified base.
    // Then we will write the base back to where it came from.
    // See the description of kDottedExpressionSetValue in Opcode.h for most information.

    auto& baseSymbolReference = dynamic_cast<SymbolReferenceExpressionNode&>(*expressionNode.base);
    emitExpression(baseSymbolReference, state);

    // We need to know:
    // - How many suffixes
    // - How many value arguments (collection indexes)
    // - How many object arguments (collection indexes)
    // In an expression like "Foo.MyList(1).MyMap(2).Field.Bar(myInt)" there are 4 suffixes, 2 value arguments, and
    // 1 object argument. We will evaluate the arguments as we go, so they're on the stack in the order they appear in
    // the suffixes. kDottedExpressionSetValue/Object will consume them.

    auto numSuffixes = expressionNode.dottedSuffixes.size();
    auto numValueArgs = 0;
    auto numObjectArgs = 0;
    for (const auto& dottedSuffix : expressionNode.dottedSuffixes) {
        if (!dottedSuffix->isIndex()) {
            continue;
        }
        assert(dottedSuffix->collectionIndex.size() == 1);
        const auto& arg = dottedSuffix->collectionIndex.at(0);
        assert(arg->evaluatedType != nullptr);
        if (arg->evaluatedType->isValueType()) {
            numValueArgs++;
        } else {
            numObjectArgs++;
        }
        emitExpression(*arg, state);
    }

    // The arguments are all pushed, so we can begin emitting the kDottedExpressionSetValue/Object call.

    assert(expressionNode.evaluatedType != nullptr);
    if (expressionNode.evaluatedType->isValueType()) {
        state->dottedExpressionSetValue(numSuffixes, numValueArgs, numObjectArgs);
    } else {
        state->dottedExpressionSetObject(numSuffixes, numValueArgs, numObjectArgs);
    }

    // The opcode is followed by an encoding of the suffixes. We push the key value/objects in the same order they
    // appear in these suffixes.

    for (const auto& dottedSuffix : expressionNode.dottedSuffixes) {
        assert(dottedSuffix->evaluatedType != nullptr);
        auto suffixIsValue = dottedSuffix->evaluatedType->isValueType();
        if (dottedSuffix->isFieldAccess()) {
            auto* bound = dottedSuffix->boundParameterNode;
            assert(bound != nullptr);
            assert(
                (suffixIsValue && bound->fieldValueIndex.has_value()) ||
                (!suffixIsValue && bound->fieldObjectIndex.has_value()));
            auto index = suffixIsValue ? *bound->fieldValueIndex : *bound->fieldObjectIndex;
            state->dottedExpressionDottedSuffix(suffixIsValue, index);
        } else if (dottedSuffix->isIndex()) {
            assert(dottedSuffix->collectionIndex.size() == 1);
            const auto& arg = *dottedSuffix->collectionIndex.at(0);
            assert(arg.evaluatedType != nullptr);
            if (arg.evaluatedType->isValueType()) {
                state->dottedExpressionValueKeySuffix(suffixIsValue);
            } else {
                state->dottedExpressionObjectKeySuffix(suffixIsValue);
            }
        } else {
            assert(false);
        }
    }

    state->returnOrBranchIfError();  // it may have raised an error

    // The kDottedExpressionSetValue/Object call is complete and the modified base is on the stack. We have to move it
    // back to the target.

    const auto* decl = baseSymbolReference.boundSymbolDeclaration;
    emitSymbolReference(*decl, state, /* set */ true);
}

static void emitAssignToSymbolReferenceExpression(
    const SymbolReferenceExpressionNode& expressionNode,
    const ExpressionNode& rhs,
    ProcedureState* state) {
    // this won't be a zero-argument function call. the type checker will ensure it is a variable.
    const auto* decl = expressionNode.boundSymbolDeclaration;
    assert(decl != nullptr);
    assert(decl->getMemberType() != MemberType::kProcedure);

    emitExpression(rhs, state);
    emitSymbolReference(*decl, state, /* set */ true);
}

/*static*/ void emitAssignToExpression(
    const ExpressionNode& expressionNode,
    const ExpressionNode& rhs,
    ProcedureState* state) {
    switch (expressionNode.getExpressionType()) {
        case ExpressionType::kDotted:
            emitAssignToDottedExpression(dynamic_cast<const DottedExpressionNode&>(expressionNode), rhs, state);
            break;
        case ExpressionType::kSymbolReference:
            emitAssignToSymbolReferenceExpression(
                dynamic_cast<const SymbolReferenceExpressionNode&>(expressionNode), rhs, state);
            break;
        default:
            throw CompilerException(
                CompilerErrorCode::kInternal, "Internal error. Wrong expression type on LHS of assignment statement.",
                expressionNode.token);
    }
}

static void emitAssignStatement(const AssignStatementNode& statementNode, ProcedureState* state) {
    emitAssignToExpression(*statementNode.target, *statementNode.value, state);
}

static void emitCallStatement(const CallStatementNode& statementNode, ProcedureState* state) {
    auto numValueArgs = 0;
    auto numObjectArgs = 0;
    int maxArgs = std::numeric_limits<uint8_t>::max();
    for (const auto& arg : statementNode.arguments) {
        assert(arg->evaluatedType != nullptr);
        arg->evaluatedType->isValueType() ? numValueArgs++ : numObjectArgs++;
        emitExpression(*arg, state);
        if (numValueArgs > maxArgs || numObjectArgs > maxArgs) {
            throw CompilerException(
                CompilerErrorCode::kTooManyCallArguments, "Too many arguments in call.", statementNode.token);
        }
    }
    if (statementNode.systemCall.has_value()) {
        if (vm::getDualGenericSystemCall(*statementNode.systemCall).has_value()) {
            throw std::runtime_error("not impl");
        }
        state->syscall(Opcode::kSystemCall, *statementNode.systemCall, numValueArgs, numObjectArgs);
    } else {
        assert(statementNode.procedureIndex.has_value());
        state->call(
            Opcode::kCall, *statementNode.procedureIndex, static_cast<uint8_t>(numValueArgs),
            static_cast<uint8_t>(numObjectArgs));
    }
}

static std::string loopTypeToString(LoopType loopType) {
    switch (loopType) {
        case LoopType::kWhile:
            return "while";
        case LoopType::kDo:
            return "do";
        case LoopType::kFor:
            return "for";
        default:
            assert(false);
            throw std::runtime_error("Unknown LoopType");
    }
}

static void emitContinueStatement(const ContinueStatementNode& statementNode, ProcedureState* state) {
    if (state->loopFrames.empty()) {
        throw CompilerException(
            CompilerErrorCode::kContinueOutsideLoop,
            "\"continue\" must be inside a \"do\", \"for\", or \"while\" loop.", statementNode.token);
    }

    const auto& loopFrame = state->loopFrames.top();

    // this check should probably not be here, but rather in a separate compiler step
    if (loopFrame.loopType != statementNode.scope) {
        throw CompilerException(
            CompilerErrorCode::kContinueTypeMismatch,
            fmt::format(
                "This \"continue {}\" statement is inside a \"{}\" loop. The loop type must match.",
                loopTypeToString(statementNode.scope), loopTypeToString(loopFrame.loopType)),
            statementNode.token);
    }

    state->jump(loopFrame.continueLabelId);
}

static void emitDimListStatement(const DimListStatementNode& statementNode, ProcedureState* state) {
    const auto& listType = statementNode.evaluatedType;
    assert(listType != nullptr);
    assert(listType->kind == Kind::kList);
    assert(listType->listItemType != nullptr);
    auto isValueList = listType->listItemType->isValueType();
    auto index = *statementNode.localObjectIndex;
    if (isValueList) {
        state->syscall(Opcode::kSystemCallO, SystemCall::kValueListBuilderNew, 0, 0);
        state->setLocalObject(index);
    } else {
        state->syscall(Opcode::kSystemCallO, SystemCall::kObjectListBuilderNew, 0, 0);
        state->setLocalObject(index);
    }
    emitBody(*statementNode.body, state);
    state->pushLocalObject(index);
    if (isValueList) {
        state->syscall(Opcode::kSystemCallO, SystemCall::kValueListBuilderEnd, 0, 1);
    } else {
        state->syscall(Opcode::kSystemCallO, SystemCall::kObjectListBuilderEnd, 0, 1);
    }
    state->setLocalObject(index);
}

static void emitDimMapStatement(const DimMapStatementNode& /*statementNode*/, ProcedureState* /*state*/) {
    throw std::runtime_error("not impl");
}

static void emitYieldStatement(const YieldStatementNode& statementNode, ProcedureState* state) {
    assert(statementNode.boundCollectionDeclaration->localObjectIndex.has_value());
    state->pushLocalObject(statementNode.boundCollectionDeclaration->localObjectIndex.value());
    if (statementNode.toExpression == nullptr) {
        // List
        emitExpression(*statementNode.expression, state);
        if (statementNode.expression->evaluatedType->isValueType()) {
            state->syscall(Opcode::kSystemCall, SystemCall::kValueListBuilderAdd, 1, 1);
        } else {
            state->syscall(Opcode::kSystemCall, SystemCall::kObjectListBuilderAdd, 0, 2);
        }
    } else {
        // Map
        throw std::runtime_error("not impl");
    }
}

static void emitDefaultValue(const TypeNode& type, ProcedureState* state) {
    // initialize all value types to 0.
    if (type.isValueType()) {
        state->pushImmediateInt64(0);
        return;
    }

    // on the other hand, object types must be created with special code per-type.
    // in the future we should optimize this by creating a single empty object of each type and reusing them.
    // this would be safe because our data types are all immutable.
    switch (type.kind) {
        case Kind::kBoolean:
        case Kind::kDate:
        case Kind::kDateTime:
        case Kind::kDateTimeOffset:
        case Kind::kNumber:
        case Kind::kTimeSpan:
            // handled above already
            assert(false);
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

        case Kind::kRecord: {
            auto numValues = 0;
            auto numObjects = 0;
            for (const auto& field : type.fields) {
                emitDefaultValue(*field->type, state);
                if (field->type->isValueType()) {
                    numValues++;
                } else {
                    numObjects++;
                }
            }
            state->recordNew(numValues, numObjects);
            break;
        }

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
}

static void emitDimOrConstStatement(
    const StatementNode& statementNode,
    const ExpressionNode* value,
    ProcedureState* state) {
    assert(statementNode.evaluatedType != nullptr);
    const auto& type = *statementNode.evaluatedType;

    // if an initial value is provided, then use that
    if (value != nullptr) {
        emitExpression(*value, state);
        if (statementNode.localValueIndex.has_value()) {
            state->setLocalValue(*statementNode.localValueIndex);
        } else if (statementNode.localObjectIndex.has_value()) {
            state->setLocalObject(*statementNode.localObjectIndex);
        } else {
            throw std::runtime_error("Internal error. No local variable index found!");
        }
        return;
    }

    emitDefaultValue(type, state);
    if (type.isValueType()) {
        state->setLocalValue(*statementNode.localValueIndex);
    } else {
        state->setLocalObject(*statementNode.localObjectIndex);
    }
}

static void emitDimStatement(const DimStatementNode& statementNode, ProcedureState* state) {
    emitDimOrConstStatement(statementNode, statementNode.value.get(), state);
}

static void emitConstStatement(const ConstStatementNode& statementNode, ProcedureState* state) {
    emitDimOrConstStatement(statementNode, statementNode.value.get(), state);
}

static void emitExitStatement(const ExitStatementNode& statementNode, ProcedureState* state) {
    if (state->loopFrames.empty()) {
        throw CompilerException(
            CompilerErrorCode::kExitOutsideLoop, "\"exit\" must be inside a \"do\", \"for\", or \"while\" loop.",
            statementNode.token);
    }

    const auto& loopFrame = state->loopFrames.top();

    // this check should probably not be here, but rather in a separate compiler step
    if (loopFrame.loopType != statementNode.scope) {
        throw CompilerException(
            CompilerErrorCode::kExitTypeMismatch,
            fmt::format(
                "This \"exit {}\" statement is inside a \"{}\" loop. The loop type must match.",
                loopTypeToString(statementNode.scope), loopTypeToString(loopFrame.loopType)),
            statementNode.token);
    }

    state->jump(loopFrame.exitLabelId);
}

static void emitForEachStatement(const ForEachStatementNode& statementNode, ProcedureState* state) {
    // we need two local temp values: count and index
    assert(statementNode.getTempLocalValueCount() == 2);
    assert(statementNode.tempLocalValueIndex.has_value());
    auto countLocalValueIndex = *statementNode.tempLocalValueIndex;
    auto indexLocalValueIndex = countLocalValueIndex + 1;

    // we need one local object value: the list
    assert(statementNode.getTempLocalObjectCount() == 1);
    assert(statementNode.tempLocalObjectIndex.has_value());
    auto listLocalObjectIndex = *statementNode.tempLocalObjectIndex;

    // plus the element variable which is user-visible
    auto isValue = statementNode.getSymbolDeclarationType()->isValueType();
    assert(
        (isValue && statementNode.localValueIndex.has_value()) ||
        (!isValue && statementNode.localObjectIndex.has_value()));
    auto elementLocalValueOrObjectIndex = isValue ? *statementNode.localValueIndex : *statementNode.localObjectIndex;

    // evaluate list
    emitExpression(*statementNode.haystack, state);
    state->setLocalObject(listLocalObjectIndex);

    // count = Len(list)
    state->pushLocalObject(listLocalObjectIndex);
    state->syscall(Opcode::kSystemCallV, SystemCall::kListLen, 0, 1);
    state->setLocalValue(countLocalValueIndex);

    // index = 0
    state->pushImmediateInt64(0);
    state->setLocalValue(indexLocalValueIndex);

    // top of loop
    auto startLabel = state->labelId();
    auto endLabel = state->labelId();
    state->label(startLabel);

    // if index >= count then jump to endLabel
    state->pushLocalValue(indexLocalValueIndex);
    state->pushLocalValue(countLocalValueIndex);
    state->syscall(Opcode::kSystemCallV, SystemCall::kNumberGreaterThanEquals, 2, 0);
    state->branchIfTrue(endLabel);

    // get the element at index
    state->pushLocalObject(listLocalObjectIndex);
    state->pushLocalValue(indexLocalValueIndex);
    if (isValue) {
        state->syscall(Opcode::kSystemCallV, SystemCall::kValueListGet, 1, 1);
        state->setLocalValue(elementLocalValueOrObjectIndex);
    } else {
        state->syscall(Opcode::kSystemCallO, SystemCall::kObjectListGet, 1, 1);
        state->setLocalObject(elementLocalValueOrObjectIndex);
    }

    // run loop body
    auto continueLabel = state->labelId();
    state->loopFrames.emplace(LoopType::kFor, continueLabel, endLabel);
    emitBody(*statementNode.body, state);
    state->loopFrames.pop();

    // increment index
    state->label(continueLabel);
    state->pushLocalValue(indexLocalValueIndex);
    state->pushImmediateInt64(1);
    state->syscall(Opcode::kSystemCallV, SystemCall::kNumberAdd, 2, 0);
    state->setLocalValue(indexLocalValueIndex);

    // end of loop
    state->jump(startLabel);
    state->label(endLabel);

    // clean up
    state->clearLocalObject(listLocalObjectIndex);
    if (!isValue) {
        state->clearLocalObject(elementLocalValueOrObjectIndex);
    }
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
    auto continueLabel = state->labelId();
    state->loopFrames.emplace(LoopType::kFor, continueLabel, endLabel);
    emitBody(*statementNode.body, state);
    state->loopFrames.pop();

    // increment the counter by the step
    state->label(continueLabel);
    state->pushLocalValue(counterLocalValueIndex);
    state->pushLocalValue(stepLocalValueIndex);
    state->syscall(Opcode::kSystemCallV, SystemCall::kNumberAdd, 2, 0);
    state->setLocalValue(counterLocalValueIndex);

    // jump to topLabel
    state->jump(topLabel);

    // we will jump here when the loop is done
    state->label(endLabel);
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

static void emitRethrowStatement(ProcedureState* state) {
    state->bubbleError();
    if (state->catchLabelIds.empty()) {
        state->returnVoid();
    } else {
        state->jump(state->catchLabelIds.top());
    }
}

static void emitReturnStatement(const ReturnStatementNode& statementNode, ProcedureState* state) {
    if (statementNode.expression == nullptr) {
        state->returnVoid();
    } else {
        assert(statementNode.expression->evaluatedType != nullptr);
        emitExpression(*statementNode.expression, state);
        auto isValue = statementNode.expression->evaluatedType->isValueType();
        if (isValue) {
            state->returnValue();
        } else {
            state->returnObject();
        }
    }
}

static void emitSelectCaseStatement(const SelectCaseStatementNode& statementNode, ProcedureState* state) {
    assert(statementNode.getTempLocalValueCount() == 1);
    assert(statementNode.getTempLocalObjectCount() == 1);
    assert(statementNode.tempLocalValueIndex.has_value());
    assert(statementNode.tempLocalObjectIndex.has_value());
    assert(statementNode.expression->evaluatedType != nullptr);

    auto isValue = statementNode.expression->evaluatedType->isValueType();
    auto exprLocalValueOrObjectIndex =
        isValue ? *statementNode.tempLocalValueIndex : *statementNode.tempLocalObjectIndex;

    emitExpression(*statementNode.expression, state);
    if (isValue) {
        state->setLocalValue(exprLocalValueOrObjectIndex);
    } else {
        state->setLocalObject(exprLocalValueOrObjectIndex);
    }

    auto endSelectCaseLabel = state->labelId();

    for (const auto& caseNode : statementNode.cases) {
        // skip "case else" right now
        if (caseNode->values.empty()) {
            continue;
        }

        auto caseBodyLabel = state->labelId();
        auto endCaseLabel = state->labelId();
        for (auto& range : caseNode->values) {
            if (range->toExpression == nullptr) {
                // single value
                if (isValue) {
                    state->pushLocalValue(exprLocalValueOrObjectIndex);
                    emitExpression(*range->expression, state);
                    state->syscall(Opcode::kSystemCallV, SystemCall::kNumberEquals, 2, 0);
                } else {
                    state->pushLocalObject(exprLocalValueOrObjectIndex);
                    emitExpression(*range->expression, state);
                    state->syscall(Opcode::kSystemCallV, SystemCall::kObjectEquals, 0, 2);
                }
            } else {
                assert(isValue);

                state->pushLocalValue(exprLocalValueOrObjectIndex);
                emitExpression(*range->expression, state);
                state->syscall(Opcode::kSystemCallV, SystemCall::kNumberGreaterThanEquals, 2, 0);

                state->pushLocalValue(exprLocalValueOrObjectIndex);
                emitExpression(*range->toExpression, state);
                state->syscall(Opcode::kSystemCallV, SystemCall::kNumberLessThanEquals, 2, 0);

                state->syscall(Opcode::kSystemCallV, SystemCall::kBooleanAnd, 2, 0);
            }
            state->branchIfTrue(caseBodyLabel);
        }

        state->jump(endCaseLabel);
        state->label(caseBodyLabel);
        emitBody(*caseNode->body, state);
        state->jump(endSelectCaseLabel);
        state->label(endCaseLabel);
    }

    // if code execution falls through to here, then it was none of the cases. run "case else"
    for (const auto& caseNode : statementNode.cases) {
        if (caseNode->values.empty()) {
            emitBody(*caseNode->body, state);
        }
    }

    state->label(endSelectCaseLabel);

    // clean up
    if (!isValue) {
        state->clearLocalObject(exprLocalValueOrObjectIndex);
    }
}

static void emitThrowStatement(const ThrowStatementNode& statementNode, ProcedureState* state) {
    if (statementNode.code != nullptr) {
        emitExpression(*statementNode.code, state);
    } else {
        state->pushImmediateInt64(0);
    }
    emitExpression(*statementNode.message, state);
    state->setError();

    if (state->catchLabelIds.empty()) {
        state->returnVoid();
    } else {
        state->jump(state->catchLabelIds.top());
    }
}

static void emitTryStatement(const TryStatementNode& statementNode, ProcedureState* state) {
    auto catchLabel = state->labelId();
    auto endTryLabel = state->labelId();
    state->catchLabelIds.push(catchLabel);
    emitBody(*statementNode.tryBody, state);
    state->catchLabelIds.pop();
    state->jump(endTryLabel);
    state->label(catchLabel);
    if (statementNode.catchBody != nullptr) {
        state->clearError();
        emitBody(*statementNode.catchBody, state);
    } else {
        state->returnIfError();
    }
    state->label(endTryLabel);
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
    state->loopFrames.emplace(LoopType::kWhile, topLabel, endLabel);
    emitBody(*statementNode.body, state);
    state->loopFrames.pop();

    // jump to topLabel
    state->jump(topLabel);

    // we will jump here when the loop is done
    state->label(endLabel);
}

static void emitDoStatement(const DoStatementNode& statementNode, ProcedureState* state) {
    // loop begins here
    auto topLabel = state->labelId();
    state->label(topLabel);

    // loop body
    auto continueLabel = state->labelId();
    auto exitLabel = state->labelId();
    state->loopFrames.emplace(LoopType::kDo, continueLabel, exitLabel);
    emitBody(*statementNode.body, state);
    state->loopFrames.pop();

    // evaluate the condition, if it's true then jump to topLabel
    state->label(continueLabel);
    emitExpression(*statementNode.condition, state);
    state->branchIfTrue(topLabel);

    state->label(exitLabel);
}

static void emitPrint(const TypeNode& type, const Token& token, ProcedureState* state) {
    // object/value must already be on the stack
    switch (type.kind) {
        case Kind::kBoolean:
            state->syscall(Opcode::kSystemCallO, SystemCall::kBooleanToString, 1, 0);
            state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
            break;
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
            state->syscall(Opcode::kSystemCallO, SystemCall::kDateTimeOffsetToString, 1, 0);
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
        case Kind::kRecord: {
            state->pushImmediateUtf8("{ ");
            state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
            auto isFirst = true;
            for (const auto& field : type.fields) {
                if (!isFirst) {
                    state->pushImmediateUtf8(", ");
                    state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
                }
                isFirst = false;
                state->pushImmediateUtf8(field->name);
                state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
                state->pushImmediateUtf8(": ");
                state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
                state->duplicateObject();
                if (field->fieldValueIndex.has_value()) {
                    state->recordGetValue(*field->fieldValueIndex);
                } else if (field->fieldObjectIndex.has_value()) {
                    state->recordGetObject(*field->fieldObjectIndex);
                } else {
                    throw CompilerException(
                        CompilerErrorCode::kInternal, fmt::format("No field index assigned for \"{}\".", field->name),
                        token);
                }
                std::string beforeChar{};
                std::string afterChar{};
                switch (field->type->kind) {
                    case Kind::kBoolean:
                    case Kind::kList:
                    case Kind::kMap:
                    case Kind::kNumber:
                    case Kind::kRecord:
                        // nothing before or after
                        break;
                    case Kind::kString:
                        beforeChar = "\"";
                        afterChar = "\"";
                        break;
                    default:
                        beforeChar = "<";
                        afterChar = ">";
                        break;
                }
                if (!beforeChar.empty()) {
                    state->pushImmediateUtf8(beforeChar);
                    state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
                }
                emitPrint(*field->type, token, state);
                if (!afterChar.empty()) {
                    state->pushImmediateUtf8(afterChar);
                    state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
                }
            }
            state->pushImmediateUtf8(" }");
            state->syscall(Opcode::kSystemCall, SystemCall::kPrintString, 0, 1);
            break;
        }
        case Kind::kOptional:
            throw std::runtime_error("not impl");
        default:
            throw CompilerException(CompilerErrorCode::kInternal, "Internal error. Unknown type.", token);
    }
}

static void emitPrintStatement(const PrintStatementNode& statementNode, ProcedureState* state) {
    for (const auto& expressionNode : statementNode.expressions) {
        assert(expressionNode->evaluatedType != nullptr);
        emitExpression(*expressionNode, state);
        emitPrint(*expressionNode->evaluatedType, expressionNode->token, state);
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
            CompilerErrorCode::kInputTargetNotVariableName,
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
        case StatementType::kIf:
            emitIfStatement(dynamic_cast<const IfStatementNode&>(statementNode), state);
            break;
        case StatementType::kRethrow:
            emitRethrowStatement(state);
            break;
        case StatementType::kReturn:
            emitReturnStatement(dynamic_cast<const ReturnStatementNode&>(statementNode), state);
            break;
        case StatementType::kSelectCase:
            emitSelectCaseStatement(dynamic_cast<const SelectCaseStatementNode&>(statementNode), state);
            break;
        case StatementType::kYield:
            emitYieldStatement(dynamic_cast<const YieldStatementNode&>(statementNode), state);
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

vector<uint8_t> emit(
    const ProcedureNode& procedureNode,
    int numLocalValues,
    int numLocalObjects,
    CompiledProgram* compiledProgram) {
#ifdef DUMP_ASM
    std::cerr << "--start of emit--";
#endif
    ProcedureState state;
    int maxLocals = std::numeric_limits<uint16_t>::max();
    if (numLocalValues > maxLocals || numLocalObjects > maxLocals) {
        throw CompilerException(
            CompilerErrorCode::kTooManyLocalVariables, "Too many local variables.", procedureNode.token);
    }
    state.initLocals(static_cast<uint16_t>(numLocalValues), static_cast<uint16_t>(numLocalObjects));

    if (boost::to_lower_copy(procedureNode.name) == "main") {
        // We have to initialize any global objects that don't have initializers.
        for (const auto& globalVariable : compiledProgram->globalVariables) {
            auto* type = globalVariable->dimOrConstStatementNode->evaluatedType.get();
            assert(type != nullptr);
            if (type->isValueType() || type->kind == Kind::kString) {
                continue;
            }
            emitDefaultValue(*type, &state);
            state.setGlobalObject(globalVariable->index);
        }
    }

    emitBody(*procedureNode.body, &state);
    state.returnVoid();
#ifdef DUMP_ASM
    std::cerr << std::endl << "--end of emit--" << std::endl;
#endif
    return state.bytecode;
}

}  // namespace compiler
