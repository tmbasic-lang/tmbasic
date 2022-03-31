// uncomment to log execution to std::cerr
// #define LOG_EXECUTION

#include "Interpreter.h"
#include "CallFrame.h"
#include "Error.h"
#include "List.h"
#include "List.h"
#include "Map.h"
#include "Object.h"
#include "Opcode.h"
#include "Optional.h"
#include "Record.h"
#include "RecordBuilder.h"
#include "String.h"
#include "constants.h"
#include "date.h"
#include "systemCall.h"
#include "util/cast.h"
#include "util/decimal.h"

namespace vm {

template <typename TInt>
static TInt readInt(const std::vector<uint8_t>* vec, size_t* index) {
    TInt value = 0;
    memcpy(&value, &vec->at(*index), sizeof(TInt));
    *index += sizeof(TInt);
    return value;
}

static void pushValue(std::array<Value, kValueStackSize>* valueStack, int* vsi, const Value& value) {
    valueStack->at(*vsi) = value;
    (*vsi)++;
}

static void pushObject(
    std::array<boost::local_shared_ptr<Object>, kObjectStackSize>* objectStack,
    int* osi,
    boost::local_shared_ptr<Object>&& obj) {
    objectStack->at(*osi) = std::move(obj);
    (*osi)++;
}

static void popValue(std::array<Value, kValueStackSize>* valueStack, int* vsi) {
    assert(*vsi > 0);
    (*vsi)--;
    valueStack->at(*vsi) = {};
}

static void popObject(std::array<boost::local_shared_ptr<Object>, kObjectStackSize>* objectStack, int* osi) {
    assert(*osi > 0);
    (*osi)--;
    objectStack->at(*osi) = {};
}

static Value* valueAt(std::array<Value, kValueStackSize>* valueStack, int vsi, int offset) {
    return &valueStack->at(vsi + offset);
}

static boost::local_shared_ptr<Object>* objectAt(
    std::array<boost::local_shared_ptr<Object>, kObjectStackSize>* objectStack,
    int osi,
    int offset) {
    return &objectStack->at(osi + offset);
}

class InterpreterPrivate {
   public:
    Program* program{};
    std::istream* consoleInputStream{};
    std::ostream* consoleOutputStream{};

    std::stack<CallFrame> callStack;
    std::array<Value, kValueStackSize> valueStack;
    std::array<boost::local_shared_ptr<Object>, kObjectStackSize> objectStack;
    bool hasError = false;
    std::string errorMessage;
    Value errorCode;

    // these are a snapshot that is only updated when run() returns
    const Procedure* procedure = nullptr;
    size_t instructionIndex = 0;
    int valueStackIndex = 0;   // points at first unused, grows upwards from 0
    int objectStackIndex = 0;  // points at first unused, grows upwards from 0

    void returnFromProcedure(
        int* vsi,
        int* osi,
        const vm::Procedure** procedure,
        const std::vector<uint8_t>** instructions,
        size_t* instructionIndex) {
        assert(!callStack.empty());
        auto& callFrame = callStack.top();
        while (*vsi > callFrame.vsiArgsStart) {
            popValue(&valueStack, vsi);
        }
        while (*osi > callFrame.osiArgsStart) {
            popObject(&objectStack, osi);
        }

        *procedure = callFrame.procedure;
        *instructions = &callFrame.procedure->instructions;
        *instructionIndex = callFrame.instructionIndex;

        callStack.pop();
    }
};

Interpreter::Interpreter(Program* program, std::istream* consoleInputStream, std::ostream* consoleOutputStream)
    : _private(new InterpreterPrivate()) {
    _private->program = program;
    _private->consoleInputStream = consoleInputStream;
    _private->consoleOutputStream = consoleOutputStream;
    initSystemCalls();
}

Interpreter::~Interpreter() {
    delete _private;
}

void Interpreter::init(int procedureIndex) {
    _private->callStack = {};
    _private->procedure = _private->program->procedures.at(procedureIndex).get();
    _private->valueStackIndex = 0;
    _private->objectStackIndex = 0;

    _private->callStack.push({ nullptr, 0, 0, 0, 0, 0, false, false });

    decimal::context = decimal::IEEEContext(decimal::DECIMAL128);
}

std::optional<InterpreterError> Interpreter::getError() const {
    if (_private->hasError) {
        return InterpreterError{ _private->errorCode, _private->errorMessage };
    }
    return std::nullopt;
}

struct SetDottedExpressionState {
    InterpreterPrivate* p{};
    int* valueStackIndex{};
    int* objectStackIndex{};
    const std::vector<uint8_t>* instructions{};
    size_t* instructionIndex{};
    bool isAssigningValue{};
    Value sourceValue{};
    boost::local_shared_ptr<Object> sourceObject{};
    bool error{};
    std::string errorMessage{};
    ErrorCode errorCode{};
};

static boost::local_shared_ptr<Object> setDottedExpressionRecurse(
    SetDottedExpressionState* state,
    const boost::local_shared_ptr<Object>& base,
    int remainingSuffixes,  // includes this one
    int nextKeyValueOffset,
    int nextKeyObjectOffset) {
    assert(state != nullptr);
    auto suffixType = readInt<uint8_t>(state->instructions, state->instructionIndex);
    assert(base != nullptr);
    auto baseType = base->getObjectType();

    // Make sure to consume all of the suffix data before returning. Don't return early.

    switch (suffixType) {
        case 0x01: {
            // Record value field. We must be assigning to this value field because you can't recurse into it.
            if (remainingSuffixes != 1) {
                throw std::runtime_error("Expected end of dotted expression, but more suffixes are present.");
            }
            if (baseType != ObjectType::kRecord) {
                throw std::runtime_error("Expected assignment target to be a record.");
            }
            if (!state->isAssigningValue) {
                throw std::runtime_error("Expected assignment target to be a value.");
            }
            auto valueFieldIndex = readInt<uint16_t>(state->instructions, state->instructionIndex);
            auto& baseRecord = dynamic_cast<Record&>(*base);
            return boost::make_local_shared<Record>(baseRecord, valueFieldIndex, state->sourceValue);
        }

        case 0x02: {
            // Record object field.
            if (baseType != ObjectType::kRecord) {
                throw std::runtime_error("Expected assignment target to be a record.");
            }
            auto objectFieldIndex = readInt<uint16_t>(state->instructions, state->instructionIndex);
            auto& baseRecord = dynamic_cast<Record&>(*base);

            if (remainingSuffixes == 1) {
                // We are assigning to this object field.
                if (state->isAssigningValue) {
                    throw std::runtime_error("Expected assignment target to be an object.");
                }
                return boost::make_local_shared<Record>(baseRecord, objectFieldIndex, state->sourceObject);
            }

            // We are recursing into this object field.
            const auto& objectField = baseRecord.objects.at(objectFieldIndex);
            auto updatedObjectField = setDottedExpressionRecurse(
                state, objectField, remainingSuffixes - 1, nextKeyValueOffset, nextKeyObjectOffset);
            return boost::make_local_shared<Record>(baseRecord, objectFieldIndex, updatedObjectField);
        }

        case 0x03: {
            // Value index/key + value element
            if (remainingSuffixes != 1) {
                throw std::runtime_error("Expected end of dotted expression, but more suffixes are present.");
            }
            if (!state->isAssigningValue) {
                throw std::runtime_error("Expected assignment target to be a value.");
            }
            auto indexOrKeyValue = *valueAt(&state->p->valueStack, *state->valueStackIndex, nextKeyValueOffset++);

            if (baseType == ObjectType::kValueList) {
                // We are assigning to this value list element.
                auto index = indexOrKeyValue.getInt64();
                auto& baseValueList = dynamic_cast<ValueList&>(*base);
                if (indexOrKeyValue.num < 0 || indexOrKeyValue.num >= baseValueList.size()) {
                    state->error = true;
                    state->errorMessage = "List index out of range.";
                    state->errorCode = ErrorCode::kListIndexOutOfRange;
                    return nullptr;
                }
                return boost::make_local_shared<ValueList>(
                    baseValueList, /* insert */ false, index, state->sourceValue);
            }

            if (baseType == ObjectType::kValueToValueMap) {
                // We are assigning to this value map element.
                auto& baseMap = dynamic_cast<ValueToValueMap&>(*base);
                return boost::make_local_shared<ValueToValueMap>(baseMap, indexOrKeyValue, state->sourceValue);
            }

            throw std::runtime_error("Expected assignment target to be value list or value-value map.");
        }

        case 0x04: {
            // Value index/key + object element
            auto indexOrKeyValue = *valueAt(&state->p->valueStack, *state->valueStackIndex, nextKeyValueOffset++);
            if (baseType == ObjectType::kObjectList) {
                auto& baseObjectList = dynamic_cast<ObjectList&>(*base);
                if (indexOrKeyValue.num < 0 || indexOrKeyValue.num >= baseObjectList.size()) {
                    state->error = true;
                    state->errorMessage = "List index out of range.";
                    state->errorCode = ErrorCode::kListIndexOutOfRange;
                    return nullptr;
                }

                if (remainingSuffixes == 1) {
                    // We are assigning to this object list element.
                    if (state->isAssigningValue) {
                        throw std::runtime_error("Expected assignment target to be an object.");
                    }
                    return boost::make_local_shared<ObjectList>(
                        baseObjectList, /* insert */ false, indexOrKeyValue.getInt64(), state->sourceObject);
                }

                // We are recursing into this object list element.
                auto index = indexOrKeyValue.getInt64();
                auto objectElement = baseObjectList.items.at(index);
                auto updatedObjectElement = setDottedExpressionRecurse(
                    state, objectElement, remainingSuffixes - 1, nextKeyValueOffset, nextKeyObjectOffset);
                return boost::make_local_shared<ObjectList>(
                    baseObjectList, /* insert */ false, indexOrKeyValue.getInt64(), updatedObjectElement);
            }

            if (baseType == ObjectType::kValueToObjectMap) {
                auto& baseMap = dynamic_cast<ValueToObjectMap&>(*base);
                if (remainingSuffixes == 1) {
                    // We are assigning to this value-object map element.
                    if (state->isAssigningValue) {
                        throw std::runtime_error("Expected assignment target to be an object.");
                    }
                    return boost::make_local_shared<ValueToObjectMap>(baseMap, indexOrKeyValue, state->sourceObject);
                }

                // We are recursing into this value-object map element.
                const auto* findResult = baseMap.pairs.find(indexOrKeyValue);
                if (findResult == nullptr) {
                    state->error = true;
                    state->errorMessage = "Map key not found.";
                    state->errorCode = ErrorCode::kMapKeyNotFound;
                    return nullptr;
                }
                auto objectElement = *findResult;  // take reference
                auto updatedObjectElement = setDottedExpressionRecurse(
                    state, objectElement, remainingSuffixes - 1, nextKeyValueOffset, nextKeyObjectOffset);
                return boost::make_local_shared<ValueToObjectMap>(baseMap, indexOrKeyValue, updatedObjectElement);
            }

            throw std::runtime_error("Expected assignment target to be object list or value-object map.");
        }

        case 0x05: {
            // Object key + value element
            if (baseType != ObjectType::kObjectToValueMap) {
                throw std::runtime_error("Expected assignment target to be an object-value map.");
            }
            if (remainingSuffixes != 1) {
                throw std::runtime_error("Expected end of dotted expression, but more suffixes are present.");
            }
            if (!state->isAssigningValue) {
                throw std::runtime_error("Expected assignment target to be an object, but it's a value.");
            }
            auto keyObject = *objectAt(&state->p->objectStack, *state->objectStackIndex, nextKeyObjectOffset++);

            // We are assigning to this object-value map element.
            auto& baseMap = dynamic_cast<ObjectToValueMap&>(*base);
            return boost::make_local_shared<ObjectToValueMap>(baseMap, keyObject, state->sourceValue);
        }

        case 0x06: {
            // Object key + object element
            if (baseType != ObjectType::kObjectToObjectMap) {
                throw std::runtime_error("Expected assignment target to be an object-object map.");
            }

            auto keyObject = *objectAt(&state->p->objectStack, *state->objectStackIndex, nextKeyObjectOffset++);

            auto& baseMap = dynamic_cast<ObjectToObjectMap&>(*base);
            if (remainingSuffixes == 1) {
                // We are assigning to this object-object map element.
                if (state->isAssigningValue) {
                    throw std::runtime_error("Expected assignment target to be an object.");
                }
                return boost::make_local_shared<ObjectToObjectMap>(baseMap, keyObject, state->sourceObject);
            }

            // We are recursing into this object-object map element.
            const auto* findResult = baseMap.pairs.find(keyObject);
            if (findResult == nullptr) {
                state->error = true;
                state->errorMessage = "Map key not found.";
                state->errorCode = ErrorCode::kMapKeyNotFound;
                return nullptr;
            }
            auto objectElement = *findResult;  // take reference
            auto updatedObjectElement = setDottedExpressionRecurse(
                state, objectElement, remainingSuffixes - 1, nextKeyValueOffset, nextKeyObjectOffset);
            return boost::make_local_shared<ObjectToObjectMap>(baseMap, keyObject, updatedObjectElement);
        }

        default:
            throw std::runtime_error("Unknown dotted expression suffix type.");
    }
}

static void setDottedExpression(SetDottedExpressionState* state) {
    assert(state->p != nullptr);
    assert(state->valueStackIndex != nullptr);
    assert(state->objectStackIndex != nullptr);
    assert(state->instructions != nullptr);
    assert(state->instructionIndex != nullptr);

    int numSuffixes = readInt<uint8_t>(state->instructions, state->instructionIndex);
    int numKeyValues = readInt<uint8_t>(state->instructions, state->instructionIndex);
    int numKeyObjects = readInt<uint8_t>(state->instructions, state->instructionIndex);

    // Let's get our bearings in the stack.
    //                  <--- lower indices              higher indices --->
    // Value stack:  [source-value]  [key-0]  [key-1]  ...  <vsi>
    // Object stack: [source-object]  [target-base]  [key-0]  [key-1]  ...  <osi>
    auto* osi = state->objectStackIndex;
    auto* vsi = state->valueStackIndex;
    auto sourceValueOffset = -numKeyValues - 1;              // If there is a source value, then it's here.
    auto startKeyValueOffset = sourceValueOffset + 1;        // Key values will start here.
    auto sourceObjectOffset = -numKeyObjects - 2;            // If there is a source object, then it's here.
    auto targetBaseObjectOffset = -numKeyObjects - 1;        // Target base object is here.
    auto startKeyObjectOffset = targetBaseObjectOffset + 1;  // Key objects will start here.

    // Some things are immediately available for us to read out.
    auto baseObject = *objectAt(&state->p->objectStack, *osi, targetBaseObjectOffset);
    assert(baseObject != nullptr);
    state->sourceValue = state->isAssigningValue ? *valueAt(&state->p->valueStack, *vsi, sourceValueOffset) : Value{};
    state->sourceObject = !state->isAssigningValue ? *objectAt(&state->p->objectStack, *osi, sourceObjectOffset)
                                                   : boost::local_shared_ptr<Object>{};

    // Figure out how long the encoded suffix data is, so that we can reliably skip past it in case
    // setDottedExpressionRecurse() fails early and doesn't consume it all.
    auto startOfSuffixes = *state->instructionIndex;
    for (auto i = 0; i < numSuffixes; i++) {
        auto suffixType = readInt<uint8_t>(state->instructions, state->instructionIndex);
        if (suffixType == 0x01 || suffixType == 0x02) {
            readInt<uint16_t>(state->instructions, state->instructionIndex);
        }
    }
    auto endInstructionIndex = *state->instructionIndex;
    *state->instructionIndex = startOfSuffixes;

    // Now recursively process the suffixes to reach the target value or object.
    auto updatedBaseObject =
        setDottedExpressionRecurse(state, baseObject, numSuffixes, startKeyValueOffset, startKeyObjectOffset);

    *state->instructionIndex = endInstructionIndex;

    // Pop the index/keys.
    for (auto i = 0; i < numKeyValues; i++) {
        popValue(&state->p->valueStack, vsi);
    }
    for (auto i = 0; i < numKeyObjects; i++) {
        popObject(&state->p->objectStack, osi);
    }

    // Pop the source value or object.
    if (state->isAssigningValue) {
        popValue(&state->p->valueStack, vsi);
    } else {
        popObject(&state->p->objectStack, osi);
    }

    // Push updatedBaseObject.
    pushObject(&state->p->objectStack, osi, std::move(updatedBaseObject));
}

bool Interpreter::run(int maxCycles) {
    const auto& procedures = _private->program->procedures;
    const auto* procedure = _private->procedure;
    const auto* instructions = &procedure->instructions;
    auto instructionIndex = _private->instructionIndex;
    auto vsi = _private->valueStackIndex;
    auto osi = _private->objectStackIndex;
    auto* valueStack = &_private->valueStack;
    auto* objectStack = &_private->objectStack;

    for (int cycle = 0; cycle < maxCycles; cycle++) {
        assert(instructions != nullptr);
        auto opcode = static_cast<Opcode>(instructions->at(instructionIndex));

#ifdef LOG_EXECUTION
        std::cerr << "cycle " << std::setw(5) << cycle << " │ pc " << std::setw(5) << instructionIndex << " │ "
                  << NAMEOF_ENUM(opcode);
        switch (opcode) {
            case Opcode::kSystemCall:
            case Opcode::kSystemCallO:
            case Opcode::kSystemCallV:
            case Opcode::kSystemCallVO: {
                int16_t syscallIndex{};
                memcpy(&syscallIndex, &instructions->at(instructionIndex + 1), sizeof(int16_t));
                std::cerr << " " << NAMEOF_ENUM(static_cast<SystemCall>(syscallIndex));
                break;
            }

            default:
                break;
        }
        std::cerr << std::endl;
#endif

        instructionIndex++;
        switch (opcode) {
            case Opcode::kExit: {
                return false;
            }

            case Opcode::kPushImmediateInt64: {
                auto imm = readInt<int64_t>(instructions, &instructionIndex);
                pushValue(valueStack, &vsi, Value{ imm });
                break;
            }

            case Opcode::kPushImmediateDec128: {
                mpd_uint128_triple_t triple;
                triple.tag = static_cast<mpd_triple_class>(readInt<uint8_t>(instructions, &instructionIndex));
                triple.sign = readInt<uint8_t>(instructions, &instructionIndex);
                triple.hi = readInt<uint64_t>(instructions, &instructionIndex);
                triple.lo = readInt<uint64_t>(instructions, &instructionIndex);
                triple.exp = readInt<int64_t>(instructions, &instructionIndex);
                pushValue(valueStack, &vsi, Value{ decimal::Decimal{ triple } });
                break;
            }

            case Opcode::kPushImmediateUtf8: {
                auto stringLength = readInt<uint32_t>(instructions, &instructionIndex);
                auto str = boost::make_local_shared<String>(&instructions->at(instructionIndex), stringLength);
                instructionIndex += stringLength;
                pushObject(objectStack, &osi, std::move(str));
                break;
            }

            case Opcode::kPopValue: {
                popValue(valueStack, &vsi);
                break;
            }

            case Opcode::kPopObject: {
                popObject(objectStack, &osi);
                break;
            }

            case Opcode::kDuplicateValue: {
                pushValue(valueStack, &vsi, valueStack->at(vsi - 1));
                break;
            }

            case Opcode::kDuplicateObject: {
                auto obj = objectStack->at(osi - 1);
                pushObject(objectStack, &osi, std::move(obj));
                break;
            }

            case Opcode::kSwapValues: {
                std::swap(valueStack->at(vsi - 1), valueStack->at(vsi - 2));
                break;
            }

            case Opcode::kSwapObjects: {
                std::swap(objectStack->at(osi - 1), objectStack->at(osi - 2));
                break;
            }

            case Opcode::kInitLocals: {
                auto numVals = readInt<uint16_t>(instructions, &instructionIndex);
                auto numObjs = readInt<uint16_t>(instructions, &instructionIndex);
                assert(!_private->callStack.empty());
                auto& frame = _private->callStack.top();
                assert(frame.vsiLocalsStart == vsi);
                assert(frame.osiLocalsStart == osi);
                (void)frame;
                vsi += numVals;
                osi += numObjs;
                break;
            }

            case Opcode::kPushArgumentValue: {
                auto argIndex = readInt<uint8_t>(instructions, &instructionIndex);
                auto& frame = _private->callStack.top();
                auto val = valueStack->at(frame.vsiArgsStart + argIndex);
                pushValue(valueStack, &vsi, val);
                break;
            }

            case Opcode::kPushArgumentObject: {
                auto argIndex = readInt<uint8_t>(instructions, &instructionIndex);
                auto& frame = _private->callStack.top();
                auto obj = objectStack->at(frame.osiArgsStart + argIndex);
                pushObject(objectStack, &osi, std::move(obj));
                break;
            }

            case Opcode::kSetArgumentValue: {
                auto argIndex = readInt<uint8_t>(instructions, &instructionIndex);
                auto value = *valueAt(valueStack, vsi, -1);
                popValue(valueStack, &vsi);
                auto& frame = _private->callStack.top();
                valueStack->at(frame.vsiArgsStart + argIndex) = value;
                break;
            }

            case Opcode::kSetArgumentObject: {
                auto argIndex = readInt<uint8_t>(instructions, &instructionIndex);
                auto obj = *objectAt(objectStack, osi, -1);
                popObject(objectStack, &osi);
                auto& frame = _private->callStack.top();
                objectStack->at(frame.osiArgsStart + argIndex) = std::move(obj);
                break;
            }

            case Opcode::kPushGlobalValue: {
                auto src = readInt<uint16_t>(instructions, &instructionIndex);
                auto val = _private->program->globalValues.at(src);
                pushValue(valueStack, &vsi, val);
                break;
            }

            case Opcode::kPushGlobalObject: {
                auto src = readInt<uint16_t>(instructions, &instructionIndex);
                auto obj = _private->program->globalObjects.at(src);
                pushObject(objectStack, &osi, std::move(obj));
                break;
            }

            case Opcode::kSetGlobalValue: {
                auto dst = readInt<uint16_t>(instructions, &instructionIndex);
                auto* val = valueAt(valueStack, vsi, -1);
                _private->program->globalValues.at(dst) = std::move(*val);
                popValue(valueStack, &vsi);
                break;
            }

            case Opcode::kSetGlobalObject: {
                auto dst = readInt<uint16_t>(instructions, &instructionIndex);
                auto* obj = objectAt(objectStack, osi, -1);
                _private->program->globalObjects.at(dst) = std::move(*obj);
                popObject(objectStack, &osi);
                break;
            }

            case Opcode::kPushLocalValue: {
                auto src = readInt<uint16_t>(instructions, &instructionIndex);
                auto& callFrame = _private->callStack.top();
                auto val = valueStack->at(callFrame.vsiLocalsStart + src);
                pushValue(valueStack, &vsi, val);
                break;
            }

            case Opcode::kPushLocalObject: {
                auto src = readInt<uint16_t>(instructions, &instructionIndex);
                auto& callFrame = _private->callStack.top();
                auto obj = objectStack->at(callFrame.osiLocalsStart + src);
                assert(obj != nullptr);
                pushObject(objectStack, &osi, std::move(obj));
                break;
            }

            case Opcode::kSetLocalValue: {
                auto dst = readInt<uint16_t>(instructions, &instructionIndex);
                auto val = *valueAt(valueStack, vsi, -1);
                auto& callFrame = _private->callStack.top();
                valueStack->at(callFrame.vsiLocalsStart + dst) = std::move(val);
                popValue(valueStack, &vsi);
                break;
            }

            case Opcode::kSetLocalObject: {
                auto dst = readInt<uint16_t>(instructions, &instructionIndex);
                auto obj = *objectAt(objectStack, osi, -1);
                assert(obj != nullptr);
                auto& callFrame = _private->callStack.top();
                objectStack->at(callFrame.osiLocalsStart + dst) = std::move(obj);
                popObject(objectStack, &osi);
                break;
            }

            case Opcode::kClearLocalObject: {
                auto dst = readInt<uint16_t>(instructions, &instructionIndex);
                auto& callFrame = _private->callStack.top();
                objectStack->at(callFrame.osiLocalsStart + dst) = nullptr;
                break;
            }

            case Opcode::kJump: {
                instructionIndex = readInt<uint32_t>(instructions, &instructionIndex);
                break;
            }

            case Opcode::kBranchIfTrue: {
                auto jumpTarget = readInt<uint32_t>(instructions, &instructionIndex);
                auto* val = valueAt(valueStack, vsi, -1);
                if (val->getBoolean()) {
                    instructionIndex = jumpTarget;
                }
                popValue(valueStack, &vsi);
                break;
            }

            case Opcode::kBranchIfFalse: {
                auto jumpTarget = readInt<uint32_t>(instructions, &instructionIndex);
                auto* val = valueAt(valueStack, vsi, -1);
                if (!val->getBoolean()) {
                    instructionIndex = jumpTarget;
                }
                popValue(valueStack, &vsi);
                break;
            }

            case Opcode::kCall:
            case Opcode::kCallV:
            case Opcode::kCallO: {
                auto procIndex = readInt<uint32_t>(instructions, &instructionIndex);
                auto numVals = readInt<uint8_t>(instructions, &instructionIndex);
                auto numObjs = readInt<uint8_t>(instructions, &instructionIndex);
                auto returnsValue = opcode == Opcode::kCallV;
                auto returnsObject = opcode == Opcode::kCallO;
                auto& callProcedure = *procedures.at(procIndex);
                _private->callStack.push(
                    { procedure, instructionIndex, numVals, numObjs, vsi, osi, returnsValue, returnsObject });
                procedure = &callProcedure;
                instructions = &callProcedure.instructions;
                assert(instructions != nullptr);
                instructionIndex = 0;
                break;
            }

            case Opcode::kSystemCall:
            case Opcode::kSystemCallV:
            case Opcode::kSystemCallO:
            case Opcode::kSystemCallVO: {
                auto syscallIndex = readInt<uint16_t>(instructions, &instructionIndex);
                auto numVals = readInt<uint8_t>(instructions, &instructionIndex);
                auto numObjs = readInt<uint8_t>(instructions, &instructionIndex);
                auto returnsValue = opcode == Opcode::kSystemCallV || opcode == Opcode::kSystemCallVO;
                auto returnsObject = opcode == Opcode::kSystemCallO || opcode == Opcode::kSystemCallVO;
                SystemCallInput systemCallInput{ valueStack,
                                                 objectStack,
                                                 vsi,
                                                 osi,
                                                 _private->consoleInputStream,
                                                 _private->consoleOutputStream,
                                                 _private->errorCode,
                                                 _private->errorMessage };
                auto result = systemCall(static_cast<SystemCall>(syscallIndex), systemCallInput);
                for (auto i = 0; i < numVals; i++) {
                    popValue(valueStack, &vsi);
                }
                for (auto i = 0; i < numObjs; i++) {
                    popObject(objectStack, &osi);
                }
                if (result.hasError) {
                    _private->hasError = result.hasError;
                    _private->errorMessage = result.errorMessage;
                    _private->errorCode.num = result.errorCode;
                } else {
                    if (returnsValue) {
                        pushValue(valueStack, &vsi, result.returnedValue);
                    }
                    if (returnsObject) {
                        assert(result.returnedObject != nullptr);
                        pushObject(objectStack, &osi, std::move(result.returnedObject));
                    }
                }
                break;
            }

            case Opcode::kReturn: {
                _private->returnFromProcedure(&vsi, &osi, &procedure, &instructions, &instructionIndex);
                if (procedure == nullptr) {
                    return false;
                }
                break;
            }

            case Opcode::kReturnValue: {
                auto val = *valueAt(valueStack, vsi, -1);
                _private->returnFromProcedure(&vsi, &osi, &procedure, &instructions, &instructionIndex);
                if (procedure == nullptr) {
                    return false;
                }
                pushValue(valueStack, &vsi, val);
                break;
            }

            case Opcode::kReturnObject: {
                auto obj = *objectAt(objectStack, osi, -1);
                _private->returnFromProcedure(&vsi, &osi, &procedure, &instructions, &instructionIndex);
                if (procedure == nullptr) {
                    return false;
                }
                pushObject(objectStack, &osi, std::move(obj));
                break;
            }

            case Opcode::kSetError: {
                const auto& message = **objectAt(objectStack, osi, -1);
                auto code = *valueAt(valueStack, vsi, -1);
                _private->hasError = true;
                _private->errorCode = code;
                _private->errorMessage = dynamic_cast<const String&>(message).toUtf8();
                popValue(valueStack, &vsi);
                popObject(objectStack, &osi);
                break;
            }

            case Opcode::kClearError: {
                _private->hasError = false;
                break;
            }

            case Opcode::kBubbleError: {
                assert(_private->errorMessage != nullptr);
                _private->hasError = true;
                break;
            }

            case Opcode::kReturnIfError: {
                if (_private->hasError) {
                    _private->returnFromProcedure(&vsi, &osi, &procedure, &instructions, &instructionIndex);
                    if (procedure == nullptr) {
                        return false;
                    }
                }
                break;
            }

            case Opcode::kBranchIfError: {
                auto jumpTarget = readInt<uint32_t>(instructions, &instructionIndex);
                if (_private->hasError) {
                    instructionIndex = jumpTarget;
                }
                break;
            }

            case Opcode::kRecordNew: {
                auto numVals = readInt<uint16_t>(instructions, &instructionIndex);
                auto numObjs = readInt<uint16_t>(instructions, &instructionIndex);
                RecordBuilder recordBuilder{ numVals, numObjs };
                for (int i = static_cast<int>(numVals) - 1; i >= 0; i--) {
                    auto val = *valueAt(valueStack, vsi, -1);
                    recordBuilder.values.set(i, std::move(val));
                    popValue(valueStack, &vsi);
                }
                for (int i = static_cast<int>(numObjs) - 1; i >= 0; i--) {
                    auto obj = *objectAt(objectStack, osi, -1);
                    recordBuilder.objects.set(i, std::move(obj));
                    popObject(objectStack, &osi);
                }
                pushObject(objectStack, &osi, boost::make_local_shared<Record>(&recordBuilder));
                break;
            }

            case Opcode::kRecordGetValue: {
                auto index = readInt<uint16_t>(instructions, &instructionIndex);
                auto& record = dynamic_cast<Record&>(**objectAt(objectStack, osi, -1));
                auto val = record.values.at(index);
                popObject(objectStack, &osi);
                pushValue(valueStack, &vsi, val);
                break;
            }

            case Opcode::kRecordGetObject: {
                auto index = readInt<uint16_t>(instructions, &instructionIndex);
                auto& record = dynamic_cast<Record&>(**objectAt(objectStack, osi, -1));
                auto obj = record.objects.at(index);
                popObject(objectStack, &osi);
                pushObject(objectStack, &osi, std::move(obj));
                break;
            }

            case Opcode::kRecordSetValue: {
                auto index = readInt<uint16_t>(instructions, &instructionIndex);
                auto& record = dynamic_cast<Record&>(**objectAt(objectStack, osi, -1));
                auto& newValue = *valueAt(valueStack, vsi, -1);
                auto newRecord = boost::make_local_shared<Record>(record, index, newValue);
                popObject(objectStack, &osi);  // pop record
                popValue(valueStack, &vsi);    // pop newValue
                pushObject(objectStack, &osi, std::move(newRecord));
                break;
            }

            case Opcode::kRecordSetObject: {
                auto index = readInt<uint16_t>(instructions, &instructionIndex);
                auto& record = dynamic_cast<Record&>(**objectAt(objectStack, osi, -2));
                auto& newObject = *objectAt(objectStack, osi, -1);
                auto newRecord = boost::make_local_shared<Record>(record, index, newObject);
                popObject(objectStack, &osi);  // pop record
                popObject(objectStack, &osi);  // pop newObject
                pushObject(objectStack, &osi, std::move(newRecord));
                break;
            }

            case Opcode::kValueListNew: {
                int numVals = readInt<uint16_t>(instructions, &instructionIndex);
                ValueListBuilder valueListBuilder{};
                for (int i = numVals - 1; i >= 0; i--) {
                    valueListBuilder.items.push_back(std::move(*valueAt(valueStack, vsi, -1 - i)));
                }
                for (int i = 0; i < numVals; i++) {
                    popValue(valueStack, &vsi);
                }
                pushObject(objectStack, &osi, boost::make_local_shared<ValueList>(&valueListBuilder));
                break;
            }

            case Opcode::kObjectListNew: {
                int numObjs = readInt<uint16_t>(instructions, &instructionIndex);
                ObjectListBuilder objectListBuilder{};
                for (int i = numObjs - 1; i >= 0; i--) {
                    objectListBuilder.items.push_back(std::move(*objectAt(objectStack, osi, -1 - i)));
                }
                for (int i = 0; i < numObjs; i++) {
                    popObject(objectStack, &osi);
                }
                pushObject(objectStack, &osi, boost::make_local_shared<ObjectList>(&objectListBuilder));
                break;
            }

            case Opcode::kDottedExpressionSetValue:
            case Opcode::kDottedExpressionSetObject: {
                SetDottedExpressionState state{};
                state.p = _private;
                state.valueStackIndex = &vsi;
                state.objectStackIndex = &osi;
                state.instructions = instructions;
                state.instructionIndex = &instructionIndex;
                state.isAssigningValue = opcode == Opcode::kDottedExpressionSetValue;
                setDottedExpression(&state);
                if (state.error) {
                    _private->hasError = true;
                    _private->errorCode.num = static_cast<int>(state.errorCode);
                    _private->errorMessage = std::move(state.errorMessage);
                }
                break;
            }

            case Opcode::kObjectToObjectMapTryGet: {
                // Input object stack: map (-2), key (-1)
                // Input value stack: (none)
                // Output object stack: element
                // Output value stack: success
                auto& map = dynamic_cast<ObjectToObjectMap&>(**objectAt(objectStack, osi, -2));
                auto& key = *objectAt(objectStack, osi, -1);
                const auto* elementWeak = map.pairs.find(key);
                auto element = elementWeak != nullptr ? *elementWeak : nullptr;  // take reference
                popObject(objectStack, &osi);
                popObject(objectStack, &osi);
                pushObject(objectStack, &osi, std::move(element));
                pushValue(valueStack, &vsi, Value{ elementWeak == nullptr ? 0 : 1 });
                break;
            }

            case Opcode::kObjectToValueMapTryGet: {
                // Input object stack: map (-2), key (-1)
                // Input value stack: (none)
                // Output object stack: (none)
                // Output value stack: element, success
                auto& map = dynamic_cast<ObjectToValueMap&>(**objectAt(objectStack, osi, -2));
                auto& key = *objectAt(objectStack, osi, -1);
                const auto* elementWeak = map.pairs.find(key);
                auto element = elementWeak != nullptr ? *elementWeak : Value{ 0 };  // copy
                popObject(objectStack, &osi);
                popObject(objectStack, &osi);
                pushValue(valueStack, &vsi, element);
                pushValue(valueStack, &vsi, Value{ elementWeak == nullptr ? 0 : 1 });
                break;
            }

            case Opcode::kValueToObjectMapTryGet: {
                // Input object stack: map (-1)
                // Input value stack: key (-1)
                // Output object stack: element
                // Output value stack: success
                auto& map = dynamic_cast<ValueToObjectMap&>(**objectAt(objectStack, osi, -1));
                auto key = *valueAt(valueStack, vsi, -1);
                const auto* elementWeak = map.pairs.find(key);
                auto element = elementWeak != nullptr ? *elementWeak : nullptr;  // take reference
                popObject(objectStack, &osi);
                popValue(valueStack, &vsi);
                pushObject(objectStack, &osi, std::move(element));
                pushValue(valueStack, &vsi, Value{ elementWeak == nullptr ? 0 : 1 });
                break;
            }

            case Opcode::kValueToValueMapTryGet: {
                // Input object stack: map (-1)
                // Input value stack: key (-1)
                // Output object stack: (none)
                // Output value stack: element, success
                auto& map = dynamic_cast<ValueToValueMap&>(**objectAt(objectStack, osi, -1));
                auto key = *valueAt(valueStack, vsi, -1);
                const auto* elementWeak = map.pairs.find(key);
                auto element = elementWeak != nullptr ? *elementWeak : Value{ 0 };  // copy
                popObject(objectStack, &osi);
                popValue(valueStack, &vsi);
                pushValue(valueStack, &vsi, element);
                pushValue(valueStack, &vsi, Value{ elementWeak == nullptr ? 0 : 1 });
                break;
            }

            case Opcode::kSetErrorMapKeyNotFound: {
                _private->hasError = true;
                _private->errorCode.num = static_cast<int>(ErrorCode::kMapKeyNotFound);
                _private->errorMessage = "Key not found.";
                break;
            }

            default:
                throw std::runtime_error(fmt::format("Unknown opcode {}", static_cast<int>(opcode)));
        }
    }

    // write state back to memory for the next run call
    _private->procedure = procedure;
    _private->instructionIndex = instructionIndex;
    _private->valueStackIndex = vsi;
    _private->objectStackIndex = osi;

    return true;
}

}  // namespace vm
