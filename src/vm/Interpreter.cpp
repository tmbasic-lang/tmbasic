#include "Interpreter.h"
#include "List.h"
#include "Map.h"
#include "Opcode.h"
#include "Record.h"
#include "String.h"

using namespace vm;

static uint16_t ReadUint16(const uint8_t* ptr);
static uint32_t ReadUint32(const uint8_t* ptr);
static int64_t ReadInt64(const uint8_t* ptr);
static int16_t ReadInt16(const uint8_t* ptr);

Interpreter::Interpreter(const Program& program) : _program(program) {}

void Interpreter::init(int procedureIndex) {
    _callStack = {};
    _recordBuilderStack = {};
    _objectListBuilderStack = {};
    _valueListBuilderStack = {};
    _procedure = &*_program.procedures[procedureIndex];
    _a = {};
    _b = {};
    _x = nullptr;
    _y = nullptr;
    _z = nullptr;
    _valueStackIndex = kValueStackSize;
    _objectStackIndex = kObjectStackSize;
}

bool Interpreter::run(int maxCycles) {
    // use local variables for our registers with the hope that the compiler chooses to put them into actual cpu
    // registers and avoids writing them to memory
    auto procedure = _procedure;
    auto instructions = procedure->artifact.value()->instructions;
    auto pc = _instruction;
    auto a = _a;
    auto b = _b;
    auto x = _x;
    auto y = _y;
    auto z = _z;
    auto vsi = _valueStackIndex;
    auto osi = _objectStackIndex;

    // if we have a fatal interpreter error (akin to a program crash) then set the error here so it can be thrown
    // after saving our state
    const char* fatalError = nullptr;

    bool more = true;

    for (int cycle = 0; cycle < maxCycles; cycle++) {
        switch (static_cast<Opcode>(*pc)) {
            case Opcode::kExit:
                pc++;
                more = false;
                goto end;

            case Opcode::kLoadConstantA:
                // ABBBBBBBB; A: opcode, B: constant
                a.integer = ReadInt64(&pc[1]);
                pc += /*A*/ 1 + /*B*/ 8;
                break;

            case Opcode::kLoadConstantB:
                // ABBBBBBBB; A: opcode, B: constant
                b.integer = ReadInt64(&pc[1]);
                pc += /*A*/ 1 + /*B*/ 8;
                break;

            case Opcode::kLoadConstantStringX: {
                // ABBBBC...C; A: opcode,  B: string length (no NUL), C: string (no NUL)
                auto stringLength = ReadUint32(&pc[1]);
                x = boost::make_local_shared<String>(&pc[5], stringLength);
                pc += /*A*/ 1 + /*B*/ 4 + /*C*/ stringLength;
                break;
            }

            case Opcode::kLoadConstantStringY: {
                // ABBBBC...C; A: opcode,  B: string length (no NUL), C: string (no NUL)
                auto stringLength = ReadUint32(&pc[1]);
                y = boost::make_local_shared<String>(&pc[5], stringLength);
                pc += /*A*/ 1 + /*B*/ 4 + /*C*/ stringLength;
                break;
            }

            case Opcode::kStoreA: {
                // ABB; A: opcode, B: index
                auto index = vsi + ReadInt16(&pc[1]);
                assert(index >= 0);
                assert(index < kValueStackSize);
                _valueStack[vsi + index] = a;
                pc += /*A*/ 1 + /*B*/ 2;
                break;
            }

            case Opcode::kStoreB: {
                // ABB; A: opcode, B: index
                auto index = vsi + ReadInt16(&pc[1]);
                assert(index >= 0);
                assert(index < kValueStackSize);
                _valueStack[vsi + index] = b;
                pc += /*A*/ 1 + /*B*/ 2;
                break;
            }

            case Opcode::kStoreX: {
                // ABB; A: opcode, B: index
                auto index = osi + ReadInt16(&pc[1]);
                assert(index >= 0);
                assert(index < kObjectStackSize);
                _objectStack[osi + index] = x;
                pc += /*A*/ 1 + /*B*/ 2;
                break;
            }

            case Opcode::kStoreY: {
                // ABB; A: opcode, B: index
                auto index = osi + ReadInt16(&pc[1]);
                assert(index >= 0);
                assert(index < kObjectStackSize);
                _objectStack[osi + index] = y;
                pc += /*A*/ 1 + /*B*/ 2;
                break;
            }

            case Opcode::kLoadA: {
                // ABB; A: opcode, B: index
                auto index = vsi + ReadInt16(&pc[1]);
                assert(index >= 0);
                assert(index < kValueStackSize);
                a = _valueStack[vsi + index];
                pc += /*A*/ 1 + /*B*/ 2;
                break;
            }

            case Opcode::kLoadB: {
                // ABB; A: opcode, B: index
                auto index = vsi + ReadInt16(&pc[1]);
                assert(index >= 0);
                assert(index < kValueStackSize);
                b = _valueStack[vsi + index];
                pc += /*A*/ 1 + /*B*/ 2;
                break;
            }

            case Opcode::kLoadX: {
                // ABB; A: opcode, B: index
                auto index = vsi + ReadInt16(&pc[1]);
                assert(index >= 0);
                assert(index < kObjectStackSize);
                x = _objectStack[osi + index];
                pc += /*A*/ 1 + /*B*/ 2;
                break;
            }

            case Opcode::kLoadY: {
                // ABB; A: opcode, B: index
                auto index = vsi + ReadInt16(&pc[1]);
                assert(index >= 0);
                assert(index < kObjectStackSize);
                y = _objectStack[osi + index];
                pc += /*A*/ 1 + /*B*/ 2;
                break;
            }

            case Opcode::kPushA:
                vsi--;
                if (vsi < 0) {
                    fatalError = "Stack overflow";
                    goto end;
                }
                _valueStack[vsi] = a;
                pc++;
                break;

            case Opcode::kPushB:
                vsi--;
                if (vsi < 0) {
                    fatalError = "Stack overflow";
                    goto end;
                }
                _valueStack[vsi] = b;
                pc++;
                break;

            case Opcode::kPushX:
                osi--;
                if (osi < 0) {
                    fatalError = "Stack overflow";
                    goto end;
                }
                _objectStack[osi] = x;
                pc++;
                break;

            case Opcode::kPushY:
                osi--;
                if (osi < 0) {
                    fatalError = "Stack overflow";
                    goto end;
                }
                _objectStack[osi] = y;
                pc++;
                break;

            case Opcode::kPopA:
                a = _valueStack[vsi];
                _valueStack[vsi].bits = 0;
                vsi++;
                assert(vsi < kValueStackSize);
                pc++;
                break;

            case Opcode::kPopB:
                b = _valueStack[vsi];
                _valueStack[vsi].bits = 0;
                vsi++;
                assert(vsi < kValueStackSize);
                pc++;
                break;

            case Opcode::kPopX:
                x.swap(_objectStack[osi]);
                _objectStack[osi] = nullptr;
                osi++;
                assert(osi < kObjectStackSize);
                pc++;
                break;

            case Opcode::kPopY:
                y.swap(_objectStack[osi]);
                _objectStack[osi] = nullptr;
                osi++;
                assert(osi < kObjectStackSize);
                pc++;
                break;

            case Opcode::kPopValues: {
                // ABB; A: opcode, B: count
                auto count = ReadUint16(&pc[1]);
                auto endIndex = vsi + count;
                assert(endIndex < kValueStackSize);
                memset(&_valueStack[vsi], 0, sizeof(Value) * count);
                vsi = endIndex;
                pc += /*A*/ 1 + /*B*/ 2;
                break;
            }

            case Opcode::kPopObjects: {
                // ABB; A: opcode, B: count
                auto count = ReadUint16(&pc[1]);
                auto endIndex = osi + count;
                assert(endIndex < kObjectStackSize);
                for (int i = 0; i < count; i++) {
                    _objectStack[osi + i] = nullptr;
                }
                osi = endIndex;
                pc += /*A*/ 1 + /*B*/ 2;
                break;
            }

            case Opcode::kClearX:
                x = nullptr;
                pc++;
                break;

            case Opcode::kClearY:
                y = nullptr;
                pc++;
                break;

            case Opcode::kSetAFromB:
                a = b;
                pc++;
                break;

            case Opcode::kSetBFromA:
                b = a;
                pc++;
                break;

            case Opcode::kSetXFromY:
                x = y;
                pc++;
                break;

            case Opcode::kSetYFromX:
                y = x;
                pc++;
                break;

            case Opcode::kAOrB:
                a.boolean = a.boolean || b.boolean;
                pc++;
                break;

            case Opcode::kAAndB:
                a.boolean = a.boolean && b.boolean;
                pc++;
                break;

            case Opcode::kAEqualsB:
                a.boolean = a.bits == b.bits;
                pc++;
                break;

            case Opcode::kANotEqualsB:
                a.boolean = a.bits != b.bits;
                pc++;
                break;

            case Opcode::kALessThanB:
                a.boolean = a.integer < b.integer;
                pc++;
                break;

            case Opcode::kALessThanEqualsB:
                a.boolean = a.integer <= b.integer;
                pc++;
                break;

            case Opcode::kAGreaterThanB:
                a.boolean = a.integer > b.integer;
                pc++;
                break;

            case Opcode::kAGreaterThanEqualsB:
                a.boolean = a.integer >= b.integer;
                pc++;
                break;

            case Opcode::kAAddB:
                a.integer += b.integer;
                pc++;
                break;

            case Opcode::kASubtractB:
                a.integer -= b.integer;
                pc++;
                break;

            case Opcode::kAMultiplyB:
                a.integer *= b.integer;
                pc++;
                break;

            case Opcode::kADivideB:
                a.integer /= b.integer;
                pc++;
                break;

            case Opcode::kAModuloB:
                a.integer %= b.integer;
                pc++;
                break;

            case Opcode::kAEqualsConstant: {
                // ABBBBBBBB; A: opcode, B: int64 constant
                int64_t constant = ReadInt64(&pc[1]);
                a.boolean = a.integer == constant;
                pc += /*A*/ 1 + /*B*/ 8;
                break;
            }

            case Opcode::kBEqualsConstant: {
                // ABBBBBBBB; A: opcode, B: int64 constant
                int64_t constant = ReadInt64(&pc[1]);
                a.boolean = b.integer == constant;
                pc += /*A*/ 1 + /*B*/ 8;
                break;
            }

            case Opcode::kStringXEqualsY:
                assert(x != nullptr);
                assert(y != nullptr);
                assert(x->getKind() == Kind::kString);
                assert(y->getKind() == Kind::kString);
                a.boolean = static_cast<String&>(*x).value == static_cast<String&>(*y).value;
                pc++;
                break;

            case Opcode::kStringXConcatenateY:
                assert(x != nullptr);
                assert(y != nullptr);
                assert(x->getKind() == Kind::kString);
                assert(y->getKind() == Kind::kString);
                x = boost::make_local_shared<String>(static_cast<String&>(*x).value + static_cast<String&>(*y).value);
                pc++;
                break;

            case Opcode::kJump: {
                // ABBBB; A: opcode, B: index
                auto index = ReadUint32(&pc[1]);
                assert(index < instructions.size());
                pc = &instructions[index];
                break;
            }

            case Opcode::kBranchIfA:
                // ABBBB; A: opcode, B: index
                if (a.boolean) {
                    auto index = ReadUint32(&pc[1]);
                    assert(index < instructions.size());
                    pc = &instructions[index];
                } else {
                    pc += /*A*/ 1 + /*B*/ 4;
                }
                break;

            case Opcode::kBranchIfNotA:
                // ABBBB; A: opcode, B: index
                if (!a.boolean) {
                    auto index = ReadUint32(&pc[1]);
                    assert(index < instructions.size());
                    pc = &instructions[index];
                } else {
                    pc += /*A*/ 1 + /*B*/ 4;
                }
                break;

            case Opcode::kCall: {
                // ABBCD
                // A: opcode
                // B: procedure index
                // C: num value arguments
                // D: num object arguments
                auto callProcedureIndex = ReadUint16(&pc[1]);
                assert(callProcedureIndex >= 0);
                assert(callProcedureIndex < _program.procedures.size());
                auto& callProcedure = *_program.procedures[callProcedureIndex];
                if (!callProcedure.artifact.has_value()) {
                    fatalError = "Call to procedure that contains an error";
                    goto end;
                }
                auto& callProcedureArtifact = *callProcedure.artifact.value();
                auto callNumValues = pc[3];
                auto callNumObjects = pc[4];
                assert(callNumValues == callProcedureArtifact.numValueParameters);
                assert(callNumObjects == callProcedureArtifact.numObjectParameters);
                pc += /*A*/ 1 + /*B*/ 2 + /*C*/ 1 + /*D*/ 1;
                _callStack.push(CallFrame(procedure, pc, vsi, osi));
                procedure = &callProcedure;
                instructions = callProcedureArtifact.instructions;
                pc = &instructions[0];
                break;
            }

            case Opcode::kReturn: {
                auto returnResult = returnFromProcedure(vsi, osi);
                procedure = returnResult.procedure;
                instructions = returnResult.instructions;
                pc = returnResult.pc;
                vsi = returnResult.valueStackIndex;
                osi = returnResult.objectStackIndex;
                break;
            }

            case Opcode::kSetError:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kString);
                _errorMessage = x;
                _errorCode = a;
                _hasError = true;
                pc++;
                break;

            case Opcode::kClearError:
                _errorMessage = nullptr;
                _errorCode.bits = 0;
                _hasError = false;
                pc++;
                break;

            case Opcode::kBubbleError:
                assert(_errorMessage != nullptr);
                _hasError = true;
                pc++;
                break;

            case Opcode::kReturnIfError:
                if (_hasError) {
                    auto returnResult = returnFromProcedure(vsi, osi);
                    procedure = returnResult.procedure;
                    instructions = returnResult.instructions;
                    pc = returnResult.pc;
                    vsi = returnResult.valueStackIndex;
                    osi = returnResult.objectStackIndex;
                } else {
                    pc++;
                }
                break;

            case Opcode::kPopBranchIfError:
                // ABBCCDDDD; A: opcode, B: num values, C: num objects, D: index
                if (_hasError) {
                    auto popValues = ReadUint16(&pc[1]);
                    if (popValues > 0) {
                        auto endIndex = vsi + popValues;
                        assert(endIndex < kValueStackSize);
                        memset(&_valueStack[vsi], 0, sizeof(Value) * popValues);
                        vsi = endIndex;
                    }
                    auto popObjects = ReadUint16(&pc[3]);
                    if (popObjects > 0) {
                        auto endIndex = osi + popObjects;
                        assert(endIndex < kObjectStackSize);
                        for (int i = 0; i < popObjects; i++) {
                            _objectStack[osi + i] = nullptr;
                        }
                        osi = endIndex;
                    }
                    auto jumpTarget = ReadUint32(&pc[5]);
                    assert(jumpTarget < instructions.size());
                    pc = &instructions[jumpTarget];
                } else {
                    pc += /*A*/ 1 + /*B*/ 2 + /*C*/ 2 + /*D*/ 4;
                }
                break;

            case Opcode::kBranchIfNotError:
                // ABBBB; A: opcode, B: index
                if (_hasError) {
                    auto index = ReadUint32(&pc[1]);
                    pc = &instructions[index];
                } else {
                    pc += /*A*/ 1 + /*B*/ 4;
                }
                break;

            case Opcode::kLoadErrorMessageX:
                assert(_errorMessage != nullptr);
                x = _errorMessage;
                pc++;
                break;

            case Opcode::kLoadErrorCodeA:
                a = _errorCode;
                pc++;
                break;

            case Opcode::kRecordBuilderBegin: {
                // ABBCC; A: opcode, B: num values, C: num objects
                auto numValues = ReadUint16(&pc[1]);
                auto numObjects = ReadUint16(&pc[3]);
                _recordBuilderStack.push(std::move(RecordBuilder(numValues, numObjects)));
                pc += /*A*/ 1 + /*B*/ 2 + /*C*/ 2;
                break;
            }

            case Opcode::kRecordBuilderStoreA: {
                // ABB; A: opcode, B: index
                assert(!_recordBuilderStack.empty());
                auto& recordBuilder = _recordBuilderStack.top();
                auto index = ReadUint16(&pc[1]);
                assert(index < recordBuilder.values.size());
                recordBuilder.values.set(index, a);
                pc += /*A*/ 1 + /*B*/ 2;
                break;
            }

            case Opcode::kRecordBuilderStoreX: {
                // ABB; A: opcode, B: index
                assert(!_recordBuilderStack.empty());
                auto& recordBuilder = _recordBuilderStack.top();
                auto index = ReadUint16(&pc[1]);
                assert(index < recordBuilder.objects.size());
                assert(x != nullptr);
                recordBuilder.objects.set(index, x);
                pc += /*A*/ 1 + /*B*/ 2;
                break;
            }

            case Opcode::kRecordBuilderEnd:
                assert(!_recordBuilderStack.empty());
                x = boost::make_local_shared<Record>(_recordBuilderStack.top());
                _recordBuilderStack.pop();
                pc++;
                break;

            case Opcode::kRecordLoadA: {
                // ABB; A: opcode, B: index
                assert(x != nullptr);
                assert(x->getKind() == Kind::kRecord);
                auto index = ReadUint16(&pc[1]);
                auto& record = static_cast<Record&>(*x);
                assert(index < record.values.size());
                a = record.values[index];
                pc += /*A*/ 1 + /*B*/ 2;
                break;
            }

            case Opcode::kRecordLoadX: {
                // ABB; A: opcode, B: index
                assert(x != nullptr);
                assert(x->getKind() == Kind::kRecord);
                auto index = ReadUint16(&pc[1]);
                auto& record = static_cast<Record&>(*x);
                assert(index < record.objects.size());
                x = record.objects[index];
                pc += /*A*/ 1 + /*B*/ 2;
                break;
            }

            case Opcode::kRecordStoreA: {
                // ABB; A: opcode, B: index
                assert(x != nullptr);
                assert(x->getKind() == Kind::kRecord);
                auto index = ReadUint16(&pc[1]);
                auto& record = static_cast<Record&>(*x);
                assert(index < record.values.size());
                x = boost::make_local_shared<Record>(record, index, a);
                pc += /*A*/ 1 + /*B*/ 2;
                break;
            }

            case Opcode::kRecordStoreY: {
                // ABB; A: opcode, B: index
                assert(x != nullptr);
                assert(x->getKind() == Kind::kRecord);
                auto index = ReadUint16(&pc[1]);
                auto& record = static_cast<Record&>(*x);
                assert(index < record.objects.size());
                assert(y != nullptr);
                x = boost::make_local_shared<Record>(record, index, y);
                pc += /*A*/ 1 + /*B*/ 2;
                break;
            }

            case Opcode::kValueListBuilderBegin: {
                _valueListBuilderStack.push(std::move(ValueListBuilder()));
                pc++;
                break;
            }

            case Opcode::kValueListBuilderAddA:
                assert(!_valueListBuilderStack.empty());
                _valueListBuilderStack.top().items.push_back(a);
                pc++;
                break;

            case Opcode::kValueListBuilderEnd:
                assert(!_valueListBuilderStack.empty());
                x = boost::make_local_shared<ValueList>(_valueListBuilderStack.top());
                _valueListBuilderStack.pop();
                pc++;
                break;

            case Opcode::kValueToValueMapNew:
                x = boost::make_local_shared<ValueToValueMap>();
                pc++;
                break;

            case Opcode::kValueToValueMapTryGet: {
                assert(x != nullptr);
                assert(x->getKind() == Kind::kValueToValueMap);
                auto found = static_cast<ValueToValueMap&>(*x).pairs.find(a);
                if (found == nullptr) {
                    b.boolean = false;
                } else {
                    b.boolean = true;
                    a = *found;
                }
                pc++;
                break;
            }

            case Opcode::kValueToValueMapCount:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kValueToValueMap);
                a.integer = static_cast<ValueToValueMap&>(*x).pairs.size();
                pc++;
                break;

            case Opcode::kValueToValueMapSet:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kValueToValueMap);
                x = boost::make_local_shared<ValueToValueMap>(static_cast<ValueToValueMap&>(*x), a, b);
                pc++;
                break;

            case Opcode::kValueToValueMapRemove:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kValueToValueMap);
                x = boost::make_local_shared<ValueToValueMap>(static_cast<ValueToValueMap&>(*x), a);
                pc++;
                break;

            case Opcode::kValueToValueMapKeys:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kValueToValueMap);
                x = static_cast<ValueToValueMap&>(*x).keys();
                pc++;
                break;

            case Opcode::kValueToValueMapValues:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kValueToValueMap);
                x = static_cast<ValueToValueMap&>(*x).values();
                pc++;
                break;

            case Opcode::kValueToObjectMapNew:
                x = boost::make_local_shared<ValueToObjectMap>();
                pc++;
                break;

            case Opcode::kValueToObjectMapTryGet: {
                assert(x != nullptr);
                assert(x->getKind() == Kind::kValueToObjectMap);
                auto found = static_cast<ValueToObjectMap&>(*x).pairs.find(a);
                if (found == nullptr) {
                    b.boolean = false;
                } else {
                    b.boolean = true;
                    x = *found;
                }
                pc++;
                break;
            }

            case Opcode::kValueToObjectMapCount:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kValueToObjectMap);
                a.integer = static_cast<ValueToObjectMap&>(*x).pairs.size();
                pc++;
                break;

            case Opcode::kValueToObjectMapSet:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kValueToObjectMap);
                x = boost::make_local_shared<ValueToObjectMap>(static_cast<ValueToObjectMap&>(*x), a, y);
                pc++;
                break;

            case Opcode::kValueToObjectMapRemove:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kValueToObjectMap);
                x = boost::make_local_shared<ValueToObjectMap>(static_cast<ValueToObjectMap&>(*x), a);
                pc++;
                break;

            case Opcode::kValueToObjectMapKeys:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kValueToObjectMap);
                x = static_cast<ValueToObjectMap&>(*x).keys();
                pc++;
                break;

            case Opcode::kValueToObjectMapValues:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kValueToObjectMap);
                x = static_cast<ValueToObjectMap&>(*x).values();
                pc++;
                break;

            case Opcode::kObjectToValueMapNew:
                x = boost::make_local_shared<ObjectToValueMap>();
                pc++;
                break;

            case Opcode::kObjectToValueMapTryGet: {
                assert(x != nullptr);
                assert(x->getKind() == Kind::kObjectToValueMap);
                auto found = static_cast<ObjectToValueMap&>(*x).pairs.find(y);
                if (found == nullptr) {
                    b.boolean = false;
                } else {
                    b.boolean = true;
                    a = *found;
                }
                pc++;
                break;
            }

            case Opcode::kObjectToValueMapCount:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kObjectToValueMap);
                a.integer = static_cast<ObjectToValueMap&>(*x).pairs.size();
                pc++;
                break;

            case Opcode::kObjectToValueMapSet:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kObjectToValueMap);
                x = boost::make_local_shared<ObjectToValueMap>(static_cast<ObjectToValueMap&>(*x), y, a);
                pc++;
                break;

            case Opcode::kObjectToValueMapRemove:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kObjectToValueMap);
                x = boost::make_local_shared<ObjectToValueMap>(static_cast<ObjectToValueMap&>(*x), y);
                pc++;
                break;

            case Opcode::kObjectToValueMapKeys:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kObjectToValueMap);
                x = static_cast<ObjectToValueMap&>(*x).keys();
                pc++;
                break;

            case Opcode::kObjectToValueMapValues:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kObjectToValueMap);
                x = static_cast<ObjectToValueMap&>(*x).values();
                pc++;
                break;

            case Opcode::kObjectToObjectMapNew:
                x = boost::make_local_shared<ObjectToObjectMap>();
                pc++;
                break;

            case Opcode::kObjectToObjectMapTryGet: {
                assert(x != nullptr);
                assert(x->getKind() == Kind::kObjectToObjectMap);
                auto found = static_cast<ObjectToObjectMap&>(*x).pairs.find(y);
                if (found == nullptr) {
                    b.boolean = false;
                } else {
                    b.boolean = true;
                    x = *found;
                }
                pc++;
                break;
            }

            case Opcode::kObjectToObjectMapCount:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kObjectToObjectMap);
                a.integer = static_cast<ObjectToObjectMap&>(*x).pairs.size();
                pc++;
                break;

            case Opcode::kObjectToObjectMapSet:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kObjectToObjectMap);
                x = boost::make_local_shared<ObjectToObjectMap>(static_cast<ObjectToObjectMap&>(*x), y, z);
                pc++;
                break;

            case Opcode::kObjectToObjectMapRemove:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kObjectToObjectMap);
                x = boost::make_local_shared<ObjectToObjectMap>(static_cast<ObjectToObjectMap&>(*x), y);
                pc++;
                break;

            case Opcode::kObjectToObjectMapKeys:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kObjectToObjectMap);
                x = static_cast<ObjectToObjectMap&>(*x).keys();
                pc++;
                break;

            case Opcode::kObjectToObjectMapValues:
                assert(x != nullptr);
                assert(x->getKind() == Kind::kObjectToObjectMap);
                x = static_cast<ObjectToObjectMap&>(*x).values();
                pc++;
                break;

            case Opcode::kStringMid: {
                assert(x != nullptr);
                assert(x->getKind() == Kind::kString);
                auto& str = static_cast<String&>(*x);
                if (a.integer >= 0 && static_cast<size_t>(a.integer) < str.value.length() && b.integer > 0) {
                    x = boost::make_local_shared<String>(str.value.substr(a.integer, b.integer));
                } else {
                    x = boost::make_local_shared<String>(std::string());
                }
                pc++;
                break;
            }

            case Opcode::kStringIndexOf: {
                assert(x != nullptr);
                assert(x->getKind() == Kind::kString);
                assert(y != nullptr);
                assert(y->getKind() == Kind::kString);
                auto& haystack = static_cast<String&>(*x);
                auto startIndex = a.integer;
                if (static_cast<size_t>(startIndex) >= haystack.value.size()) {
                    a.integer = -1;
                } else {
                    if (startIndex < 0) {
                        startIndex = 0;
                    }
                    auto& needle = static_cast<String&>(*y);
                    auto found = haystack.value.find(needle.value, a.integer);
                    a.integer = found == std::string::npos ? -1 : found;
                }
                pc++;
                break;
            }

            case Opcode::kStringChr: {
                char ch = a.bits & 0xFF;
                x = boost::make_local_shared<String>(std::string(&ch, 1));
                pc++;
                break;
            }

            case Opcode::kStringAsc: {
                assert(x != nullptr);
                assert(x->getKind() == Kind::kString);
                auto& str = static_cast<String&>(*x);
                if (a.integer >= 0 && static_cast<size_t>(a.integer) < str.value.length()) {
                    a.integer = str.value[a.integer];
                } else {
                    a.integer = -1;
                }
                pc++;
                break;
            }

            default:
                assert(false);
        }
    }

    // write state back to memory for the next run call
end:
    _procedure = procedure;
    _instruction = pc;
    _a = a;
    _b = b;
    _x = x;
    _y = y;
    _z = z;
    _valueStackIndex = vsi;
    _objectStackIndex = osi;

    if (fatalError) {
        throw std::runtime_error(fatalError);
    }

    return more;
}

Interpreter::ReturnResult::ReturnResult(
    const Procedure* procedure,
    const std::vector<uint8_t>& instructions,
    const uint8_t* pc,
    int valueStackIndex,
    int objectStackIndex)
    : procedure(procedure),
      instructions(instructions),
      pc(pc),
      valueStackIndex(valueStackIndex),
      objectStackIndex(objectStackIndex) {}

Interpreter::ReturnResult Interpreter::returnFromProcedure(int valueStackIndex, int objectStackIndex) {
    assert(!_callStack.empty());
    auto callFrame = _callStack.top();
    assert(valueStackIndex <= callFrame.valueStackIndex);
    for (int i = valueStackIndex; i < callFrame.valueStackIndex; i++) {
        _valueStack[i].bits = 0;
    }
    assert(objectStackIndex <= callFrame.objectStackIndex);
    for (int i = objectStackIndex; i < callFrame.objectStackIndex; i++) {
        _objectStack[i] = nullptr;
    }
    _callStack.pop();
    return ReturnResult(
        callFrame.procedure, callFrame.procedure->artifact.value()->instructions, callFrame.instruction,
        callFrame.valueStackIndex, callFrame.objectStackIndex);
}

uint16_t ReadUint16(const uint8_t* ptr) {
    uint16_t value;
    memcpy(&value, ptr, sizeof(uint16_t));
    return value;
}

uint32_t ReadUint32(const uint8_t* ptr) {
    uint32_t value;
    memcpy(&value, ptr, sizeof(uint32_t));
    return value;
}

int64_t ReadInt64(const uint8_t* ptr) {
    int64_t value;
    memcpy(&value, ptr, sizeof(int64_t));
    return value;
}

int16_t ReadInt16(const uint8_t* ptr) {
    int16_t value;
    memcpy(&value, ptr, sizeof(int16_t));
    return value;
}
