#include "assemble.h"
#include "vm/Opcode.h"
#include "vm/Procedure.h"
#include "vm/Program.h"
#include "vm/systemCall.h"

using std::array;
using std::getline;
using std::istream;
using std::make_unique;
using std::move;
using std::regex;
using std::regex_search;
using std::smatch;
using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::vector;
using vm::Opcode;
using vm::Procedure;
using vm::Program;
using vm::SystemCall;

static Opcode parseOpcode(string s) {
    // generate this map using:
    // grep , src/vm/Opcode.h | sed s/,//g | awk '{ print "{\"" $1 "\",Opcode::" $1 "}," }' | sed 's/"k/"/g'
    static const std::unordered_map<string, Opcode> map = {
        { "Exit", Opcode::kExit },
        { "PushImmediateInt64", Opcode::kPushImmediateInt64 },
        { "PushImmediateUtf8", Opcode::kPushImmediateUtf8 },
        { "PopValue", Opcode::kPopValue },
        { "PopObject", Opcode::kPopObject },
        { "InitLocals", Opcode::kInitLocals },
        { "PushArgumentValue", Opcode::kPushArgumentValue },
        { "PushArgumentObject", Opcode::kPushArgumentObject },
        { "PushGlobalValue", Opcode::kPushGlobalValue },
        { "PushGlobalObject", Opcode::kPushGlobalObject },
        { "SetGlobalValue", Opcode::kSetGlobalValue },
        { "SetGlobalObject", Opcode::kSetGlobalObject },
        { "PushLocalValue", Opcode::kPushLocalValue },
        { "PushLocalObject", Opcode::kPushLocalObject },
        { "SetLocalValue", Opcode::kSetLocalValue },
        { "SetLocalObject", Opcode::kSetLocalObject },
        { "Jump", Opcode::kJump },
        { "BranchIfTrue", Opcode::kBranchIfTrue },
        { "BranchIfFalse", Opcode::kBranchIfFalse },
        { "CallSub", Opcode::kCallSub },
        { "CallFunctionValue", Opcode::kCallFunctionValue },
        { "CallFunctionObject", Opcode::kCallFunctionObject },
        { "SystemCallSub", Opcode::kSystemCallSub },
        { "SystemCallFunctionValue", Opcode::kSystemCallFunctionValue },
        { "SystemCallFunctionObject", Opcode::kSystemCallFunctionObject },
        { "SystemCallFunctionValueObject", Opcode::kSystemCallFunctionValueObject },
        { "Return", Opcode::kReturn },
        { "ReturnValue", Opcode::kReturnValue },
        { "ReturnObject", Opcode::kReturnObject },
        { "SetError", Opcode::kSetError },
        { "ClearError", Opcode::kClearError },
        { "BubbleError", Opcode::kBubbleError },
        { "ReturnIfError", Opcode::kReturnIfError },
        { "BranchIfNotError", Opcode::kBranchIfNotError },
        { "PushErrorMessage", Opcode::kPushErrorMessage },
        { "PushErrorCode", Opcode::kPushErrorCode },
        { "RecordNew", Opcode::kRecordNew },
        { "RecordGetValue", Opcode::kRecordGetValue },
        { "RecordGetObject", Opcode::kRecordGetObject },
        { "ValueListNew", Opcode::kValueListNew },
        { "ObjectListNew", Opcode::kObjectListNew },
    };

    auto it = map.find(s);
    if (it != map.end()) {
        return it->second;
    }

    std::cerr << "Unknown opcode: " << s << std::endl;
    assert(false);
    return {};
}

static SystemCall parseSystemCall(string s) {
    // generate this map using:
    /* grep "^    k" src/vm/systemCall.h | sed s/,//g | awk '{ print "{\"" $1 "\",SystemCall::" $1 "}," }' | \
     sed 's/"k/"/g'
    */
    static const std::unordered_map<string, SystemCall> map = {
        { "Add", SystemCall::kAdd },
        { "AvailableLocales", SystemCall::kAvailableLocales },
        { "AvailableTimeZones", SystemCall::kAvailableTimeZones },
        { "Characters1", SystemCall::kCharacters1 },
        { "Characters2", SystemCall::kCharacters2 },
        { "Chr", SystemCall::kChr },
        { "DateFromParts", SystemCall::kDateFromParts },
        { "DateTimeFromParts", SystemCall::kDateTimeFromParts },
        { "DateTimeOffsetFromParts", SystemCall::kDateTimeOffsetFromParts },
        { "Days", SystemCall::kDays },
        { "FlushConsoleOutput", SystemCall::kFlushConsoleOutput },
        { "HasValueO", SystemCall::kHasValueO },
        { "HasValueV", SystemCall::kHasValueV },
        { "Hours", SystemCall::kHours },
        { "Len", SystemCall::kLen },
        { "Milliseconds", SystemCall::kMilliseconds },
        { "Minutes", SystemCall::kMinutes },
        { "NumberToString", SystemCall::kNumberToString },
        { "ObjectListGet", SystemCall::kObjectListGet },
        { "ObjectListLength", SystemCall::kObjectListLength },
        { "PrintString", SystemCall::kPrintString },
        { "Seconds", SystemCall::kSeconds },
        { "TimeZoneFromName", SystemCall::kTimeZoneFromName },
        { "TotalDays", SystemCall::kTotalDays },
        { "TotalHours", SystemCall::kTotalHours },
        { "TotalMilliseconds", SystemCall::kTotalMilliseconds },
        { "TotalMinutes", SystemCall::kTotalMinutes },
        { "TotalSeconds", SystemCall::kTotalSeconds },
        { "UtcOffset", SystemCall::kUtcOffset },
        { "ValueO", SystemCall::kValueO },
        { "ValueV", SystemCall::kValueV },
    };

    auto it = map.find(s);
    if (it != map.end()) {
        return it->second;
    }

    assert(false);
    return {};
}

template <typename TInt>
static void appendInt(vector<uint8_t>* vec, TInt value) {
    array<uint8_t, sizeof(TInt)> bytes{};
    memcpy(bytes.data(), &value, sizeof(TInt));
    vec->insert(vec->end(), bytes.begin(), bytes.end());
}

template <typename TInt>
static void appendInt(vector<uint8_t>* vec, istream* input) {
    int64_t value{};
    *input >> value;
    appendInt<TInt>(vec, static_cast<TInt>(value));
}

static void appendLengthTaggedString(vector<uint8_t>* vec, istream* input) {
    string quoted;
    getline(*input, quoted);

    // strip leading and trailing whitespace and quotes
    smatch match;
    auto regex_success = regex_search(quoted, match, regex("^\\s*\"([^\"]*)\"\\s*$"));
    assert(regex_success);
    (void)regex_success;  // avoid unused variable error in release builds
    auto unquoted = match[1].str();

    appendInt(vec, static_cast<uint32_t>(unquoted.size()));
    for (auto ch : unquoted) {
        vec->push_back(static_cast<uint8_t>(ch));
    }
}

struct LabelUsage {
    string label;
    size_t index;
};

static void addNewProcedure(
    Program* program,
    vector<uint8_t>* instructions,
    unordered_map<string, size_t>* labels,
    vector<LabelUsage>* labelUsages) {
    for (const auto& labelUsage : *labelUsages) {
        auto it = labels->find(labelUsage.label);
        assert(it != labels->end());
        uint32_t jumpTarget = static_cast<uint32_t>(it->second);
        memcpy(&instructions->at(labelUsage.index), &jumpTarget, sizeof(uint32_t));
    }

    auto procedure = make_unique<Procedure>();
    procedure->instructions = move(*instructions);
    program->procedures.push_back(move(procedure));

    labelUsages->clear();
    labels->clear();
    instructions->clear();
}

std::unique_ptr<vm::Program> assemble(istream* input) {
    auto program = make_unique<Program>();
    auto vec = vector<uint8_t>();
    auto labels = unordered_map<string, size_t>();
    auto labelUsages = vector<LabelUsage>();

    while (!input->eof()) {
        string opcodeStr;
        *input >> opcodeStr;
        if (opcodeStr == "") {
            break;
        }
        if (opcodeStr == "-") {
            addNewProcedure(program.get(), &vec, &labels, &labelUsages);
            continue;
        }
        if (opcodeStr == "#") {
            string dummy;
            getline(*input, dummy);
            continue;
        }
        if (opcodeStr == "label") {
            string name;
            *input >> name;
            labels[name] = vec.size();
            continue;
        }
        auto opcode = parseOpcode(opcodeStr);
        vec.push_back(static_cast<uint8_t>(opcode));

        switch (opcode) {
            case Opcode::kPushImmediateInt64:
                appendInt<uint64_t>(&vec, input);
                break;

            case Opcode::kPushImmediateUtf8:
                appendLengthTaggedString(&vec, input);
                break;

            case Opcode::kInitLocals:
                appendInt<uint16_t>(&vec, input);
                appendInt<uint16_t>(&vec, input);
                break;

            case Opcode::kPushArgumentValue:
            case Opcode::kPushArgumentObject:
                appendInt<uint8_t>(&vec, input);
                break;

            case Opcode::kPushGlobalValue:
            case Opcode::kPushGlobalObject:
            case Opcode::kSetGlobalValue:
            case Opcode::kSetGlobalObject:
            case Opcode::kPushLocalValue:
            case Opcode::kPushLocalObject:
            case Opcode::kSetLocalValue:
            case Opcode::kSetLocalObject:
            case Opcode::kRecordGetValue:
            case Opcode::kRecordGetObject:
                appendInt<uint16_t>(&vec, input);
                break;

            case Opcode::kJump:
            case Opcode::kBranchIfTrue:
            case Opcode::kBranchIfFalse:
            case Opcode::kBranchIfNotError: {
                string label;
                *input >> label;
                labelUsages.push_back({ label, vec.size() });
                appendInt<uint32_t>(&vec, static_cast<uint32_t>(0));
                break;
            }

            case Opcode::kCallSub:
            case Opcode::kCallFunctionValue:
            case Opcode::kCallFunctionObject: {
                appendInt<uint32_t>(&vec, input);
                appendInt<uint8_t>(&vec, input);
                appendInt<uint8_t>(&vec, input);
                break;
            }

            case Opcode::kSystemCallSub:
            case Opcode::kSystemCallFunctionValue:
            case Opcode::kSystemCallFunctionObject:
            case Opcode::kSystemCallFunctionValueObject: {
                string systemCallStr;
                *input >> systemCallStr;
                appendInt<uint16_t>(&vec, static_cast<uint16_t>(parseSystemCall(systemCallStr)));
                appendInt<uint8_t>(&vec, input);
                appendInt<uint8_t>(&vec, input);
                break;
            }

            default:
                break;
        }
    }

    if (!vec.empty()) {
        addNewProcedure(program.get(), &vec, &labels, &labelUsages);
    }

    return program;
}
