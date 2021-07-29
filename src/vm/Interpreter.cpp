// uncomment to log execution to std::cerr
// #define LOG_EXECUTION

#include "Interpreter.h"
#include "List.h"
#include "Map.h"
#include "Opcode.h"
#include "Optional.h"
#include "Record.h"
#include "String.h"
#include "date.h"
#include "systemCall.h"
#include "util/cast.h"
#include "util/decimal.h"
#include "vm/CallFrame.h"
#include "vm/List.h"
#include "vm/Object.h"
#include "vm/RecordBuilder.h"
#include "vm/constants.h"

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
    (*vsi)--;
    valueStack->at(*vsi) = {};
}

static void popObject(std::array<boost::local_shared_ptr<Object>, kObjectStackSize>* objectStack, int* osi) {
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
        std::cerr << "cycle " << cycle << ": " << NAMEOF_ENUM(opcode) << std::endl;
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

            case Opcode::kInitLocals: {
                auto numVals = readInt<uint16_t>(instructions, &instructionIndex);
                auto numObjs = readInt<uint16_t>(instructions, &instructionIndex);
                assert(!_private->callStack.empty());
                auto& frame = _private->callStack.top();
                assert(frame.vsiLocalsStart == vsi);
                assert(frame.osiLocalsStart == osi);
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
                auto* val = valueAt(valueStack, vsi, -1);
                auto& callFrame = _private->callStack.top();
                valueStack->at(callFrame.vsiLocalsStart + dst) = std::move(*val);
                popValue(valueStack, &vsi);
                break;
            }

            case Opcode::kSetLocalObject: {
                auto dst = readInt<uint16_t>(instructions, &instructionIndex);
                auto* obj = objectAt(objectStack, osi, -1);
                auto& callFrame = _private->callStack.top();
                objectStack->at(callFrame.osiLocalsStart + dst) = std::move(*obj);
                popObject(objectStack, &osi);
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
                SystemCallInput systemCallInput{
                    *valueStack, *objectStack, vsi, osi, _private->consoleInputStream, _private->consoleOutputStream
                };
                auto result = systemCall(static_cast<SystemCall>(syscallIndex), systemCallInput);
                for (auto i = 0; i < numVals; i++) {
                    popValue(valueStack, &vsi);
                }
                for (auto i = 0; i < numObjs; i++) {
                    popObject(objectStack, &osi);
                }
                if (returnsValue) {
                    pushValue(valueStack, &vsi, result.returnedValue);
                }
                if (returnsObject) {
                    assert(result.returnedObject != nullptr);
                    pushObject(objectStack, &osi, std::move(result.returnedObject));
                }
                if (result.hasError) {
                    _private->hasError = result.hasError;
                    _private->errorMessage = result.errorMessage;
                    _private->errorCode.num = result.errorCode;
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

            case Opcode::kPushErrorMessage: {
                pushObject(objectStack, &osi, boost::make_local_shared<String>(_private->errorMessage));
                break;
            }

            case Opcode::kPushErrorCode: {
                pushValue(valueStack, &vsi, _private->errorCode);
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

            default:
                std::cerr << "Unimplemented opcode: " << instructions->at(instructionIndex) << std::endl;
                assert(false);
                break;
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
