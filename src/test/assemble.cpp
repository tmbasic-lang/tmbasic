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
        { "LoadConstantA", Opcode::kLoadConstantA },
        { "LoadConstantB", Opcode::kLoadConstantB },
        { "LoadConstantStringX", Opcode::kLoadConstantStringX },
        { "LoadConstantStringY", Opcode::kLoadConstantStringY },
        { "LoadConstantStringZ", Opcode::kLoadConstantStringZ },
        { "StoreA", Opcode::kStoreA },
        { "StoreB", Opcode::kStoreB },
        { "StoreX", Opcode::kStoreX },
        { "StoreY", Opcode::kStoreY },
        { "StoreZ", Opcode::kStoreZ },
        { "LoadA", Opcode::kLoadA },
        { "LoadB", Opcode::kLoadB },
        { "LoadX", Opcode::kLoadX },
        { "LoadY", Opcode::kLoadY },
        { "LoadZ", Opcode::kLoadZ },
        { "PushValues", Opcode::kPushValues },
        { "PushObjects", Opcode::kPushObjects },
        { "PopValues", Opcode::kPopValues },
        { "PopObjects", Opcode::kPopObjects },
        { "ClearX", Opcode::kClearX },
        { "ClearY", Opcode::kClearY },
        { "ClearZ", Opcode::kClearZ },
        { "SetAFromB", Opcode::kSetAFromB },
        { "SetBFromA", Opcode::kSetBFromA },
        { "SetXFromY", Opcode::kSetXFromY },
        { "SetYFromX", Opcode::kSetYFromX },
        { "AOrB", Opcode::kAOrB },
        { "AAndB", Opcode::kAAndB },
        { "AEqualsB", Opcode::kAEqualsB },
        { "ANotEqualsB", Opcode::kANotEqualsB },
        { "ALessThanB", Opcode::kALessThanB },
        { "ALessThanEqualsB", Opcode::kALessThanEqualsB },
        { "AGreaterThanB", Opcode::kAGreaterThanB },
        { "AGreaterThanEqualsB", Opcode::kAGreaterThanEqualsB },
        { "AAddB", Opcode::kAAddB },
        { "ASubtractB", Opcode::kASubtractB },
        { "AMultiplyB", Opcode::kAMultiplyB },
        { "ADivideB", Opcode::kADivideB },
        { "AModuloB", Opcode::kAModuloB },
        { "AEqualsConstant", Opcode::kAEqualsConstant },
        { "BEqualsConstant", Opcode::kBEqualsConstant },
        { "StringXEqualsY", Opcode::kStringXEqualsY },
        { "StringXConcatenateY", Opcode::kStringXConcatenateY },
        { "Jump", Opcode::kJump },
        { "BranchIfA", Opcode::kBranchIfA },
        { "BranchIfNotA", Opcode::kBranchIfNotA },
        { "Call", Opcode::kCall },
        { "SystemCall", Opcode::kSystemCall },
        { "Return", Opcode::kReturn },
        { "SetError", Opcode::kSetError },
        { "ClearError", Opcode::kClearError },
        { "BubbleError", Opcode::kBubbleError },
        { "ReturnIfError", Opcode::kReturnIfError },
        { "PopBranchIfError", Opcode::kPopBranchIfError },
        { "BranchIfNotError", Opcode::kBranchIfNotError },
        { "LoadErrorMessageX", Opcode::kLoadErrorMessageX },
        { "LoadErrorCodeA", Opcode::kLoadErrorCodeA },
        { "RecordBuilderBegin", Opcode::kRecordBuilderBegin },
        { "RecordBuilderStoreA", Opcode::kRecordBuilderStoreA },
        { "RecordBuilderStoreX", Opcode::kRecordBuilderStoreX },
        { "RecordBuilderEnd", Opcode::kRecordBuilderEnd },
        { "RecordLoadA", Opcode::kRecordLoadA },
        { "RecordLoadX", Opcode::kRecordLoadX },
        { "RecordStoreA", Opcode::kRecordStoreA },
        { "RecordStoreY", Opcode::kRecordStoreY },
        { "ValueListBuilderBegin", Opcode::kValueListBuilderBegin },
        { "ValueListBuilderAddA", Opcode::kValueListBuilderAddA },
        { "ValueListBuilderEnd", Opcode::kValueListBuilderEnd },
        { "ValueListGet", Opcode::kValueListGet },
        { "ValueListSet", Opcode::kValueListSet },
        { "ValueListCount", Opcode::kValueListCount },
        { "ValueListInsert", Opcode::kValueListInsert },
        { "ValueListRemove", Opcode::kValueListRemove },
        { "ObjectListBuilderBegin", Opcode::kObjectListBuilderBegin },
        { "ObjectListBuilderAddX", Opcode::kObjectListBuilderAddX },
        { "ObjectListBuilderEnd", Opcode::kObjectListBuilderEnd },
        { "ObjectListGet", Opcode::kObjectListGet },
        { "ObjectListSet", Opcode::kObjectListSet },
        { "ObjectListCount", Opcode::kObjectListCount },
        { "ObjectListInsert", Opcode::kObjectListInsert },
        { "ObjectListRemove", Opcode::kObjectListRemove },
        { "ValueToValueMapNew", Opcode::kValueToValueMapNew },
        { "ValueToValueMapTryGet", Opcode::kValueToValueMapTryGet },
        { "ValueToValueMapCount", Opcode::kValueToValueMapCount },
        { "ValueToValueMapSet", Opcode::kValueToValueMapSet },
        { "ValueToValueMapRemove", Opcode::kValueToValueMapRemove },
        { "ValueToValueMapKeys", Opcode::kValueToValueMapKeys },
        { "ValueToValueMapValues", Opcode::kValueToValueMapValues },
        { "ValueToObjectMapNew", Opcode::kValueToObjectMapNew },
        { "ValueToObjectMapTryGet", Opcode::kValueToObjectMapTryGet },
        { "ValueToObjectMapCount", Opcode::kValueToObjectMapCount },
        { "ValueToObjectMapSet", Opcode::kValueToObjectMapSet },
        { "ValueToObjectMapRemove", Opcode::kValueToObjectMapRemove },
        { "ValueToObjectMapKeys", Opcode::kValueToObjectMapKeys },
        { "ValueToObjectMapValues", Opcode::kValueToObjectMapValues },
        { "ObjectToValueMapNew", Opcode::kObjectToValueMapNew },
        { "ObjectToValueMapTryGet", Opcode::kObjectToValueMapTryGet },
        { "ObjectToValueMapCount", Opcode::kObjectToValueMapCount },
        { "ObjectToValueMapSet", Opcode::kObjectToValueMapSet },
        { "ObjectToValueMapRemove", Opcode::kObjectToValueMapRemove },
        { "ObjectToValueMapKeys", Opcode::kObjectToValueMapKeys },
        { "ObjectToValueMapValues", Opcode::kObjectToValueMapValues },
        { "ObjectToObjectMapNew", Opcode::kObjectToObjectMapNew },
        { "ObjectToObjectMapTryGet", Opcode::kObjectToObjectMapTryGet },
        { "ObjectToObjectMapCount", Opcode::kObjectToObjectMapCount },
        { "ObjectToObjectMapSet", Opcode::kObjectToObjectMapSet },
        { "ObjectToObjectMapRemove", Opcode::kObjectToObjectMapRemove },
        { "ObjectToObjectMapKeys", Opcode::kObjectToObjectMapKeys },
        { "ObjectToObjectMapValues", Opcode::kObjectToObjectMapValues },
        { "ValueOptionalNewMissing", Opcode::kValueOptionalNewMissing },
        { "ValueOptionalNewPresent", Opcode::kValueOptionalNewPresent },
        { "ObjectOptionalNewMissing", Opcode::kObjectOptionalNewMissing },
        { "ObjectOptionalNewPresent", Opcode::kObjectOptionalNewPresent },
        { "ValueGlobalStore", Opcode::kValueGlobalStore },
        { "ValueGlobalLoad", Opcode::kValueGlobalLoad },
        { "ObjectGlobalStore", Opcode::kObjectGlobalStore },
        { "ObjectGlobalLoad", Opcode::kObjectGlobalLoad },
        { "StringMid", Opcode::kStringMid },
        { "StringIndexOf", Opcode::kStringIndexOf },
        { "StringAsc", Opcode::kStringAsc },
        { "StringPrint", Opcode::kStringPrint },
        { "StringInputLine", Opcode::kStringInputLine },
        { "NumberToString", Opcode::kNumberToString },
        { "StringToNumber", Opcode::kStringToNumber },
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
        { "AvailableLocales", SystemCall::kAvailableLocales },
        { "AvailableTimeZones", SystemCall::kAvailableTimeZones },
        { "Characters1", SystemCall::kCharacters1 },
        { "Characters2", SystemCall::kCharacters2 },
        { "Chr", SystemCall::kChr },
        { "DateFromParts", SystemCall::kDateFromParts },
        { "DateTimeFromParts", SystemCall::kDateTimeFromParts },
        { "DateTimeOffsetFromParts", SystemCall::kDateTimeOffsetFromParts },
        { "Days", SystemCall::kDays },
        { "HasValueO", SystemCall::kHasValueO },
        { "HasValueV", SystemCall::kHasValueV },
        { "Hours", SystemCall::kHours },
        { "Len", SystemCall::kLen },
        { "Milliseconds", SystemCall::kMilliseconds },
        { "Minutes", SystemCall::kMinutes },
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

static void appendInt64(vector<uint8_t>* vec, istream* input) {
    int64_t value = 0;
    *input >> value;
    auto bytes = array<uint8_t, 8>();
    memcpy(bytes.data(), &value, 8);
    for (auto b : bytes) {
        vec->push_back(b);
    }
}

static void appendUint16(vector<uint8_t>* vec, istream* input) {
    uint16_t value = 0;
    *input >> value;
    auto bytes = array<uint8_t, 2>();
    memcpy(bytes.data(), &value, 2);
    for (auto b : bytes) {
        vec->push_back(b);
    }
}

static void appendUint16(vector<uint8_t>* vec, uint16_t value) {
    auto bytes = array<uint16_t, 2>();
    memcpy(bytes.data(), &value, 2);
    for (auto b : bytes) {
        vec->push_back(b);
    }
}

static void appendUint32(vector<uint8_t>* vec, uint32_t value) {
    auto bytes = array<uint8_t, 4>();
    memcpy(bytes.data(), &value, 4);
    for (auto b : bytes) {
        vec->push_back(b);
    }
}

static void appendUint32(vector<uint8_t>* vec, istream* input) {
    uint32_t value = 0;
    *input >> value;
    appendUint32(vec, value);
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

    appendUint32(vec, static_cast<uint32_t>(unquoted.size()));
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
            case Opcode::kLoadConstantA:
            case Opcode::kLoadConstantB:
            case Opcode::kAEqualsConstant:
            case Opcode::kBEqualsConstant:
                appendInt64(&vec, input);
                break;

            case Opcode::kLoadConstantStringX:
            case Opcode::kLoadConstantStringY:
            case Opcode::kLoadConstantStringZ:
                appendLengthTaggedString(&vec, input);
                break;

            case Opcode::kStoreA:
            case Opcode::kStoreB:
            case Opcode::kStoreX:
            case Opcode::kStoreY:
            case Opcode::kStoreZ:
            case Opcode::kLoadA:
            case Opcode::kLoadB:
            case Opcode::kLoadX:
            case Opcode::kLoadY:
            case Opcode::kLoadZ:
            case Opcode::kPopValues:
            case Opcode::kPopObjects:
            case Opcode::kPushValues:
            case Opcode::kPushObjects:
            case Opcode::kRecordBuilderStoreA:
            case Opcode::kRecordBuilderStoreX:
            case Opcode::kRecordLoadA:
            case Opcode::kRecordLoadX:
            case Opcode::kRecordStoreA:
            case Opcode::kRecordStoreY:
            case Opcode::kCall:
                appendUint16(&vec, input);
                break;

            case Opcode::kJump:
            case Opcode::kBranchIfA:
            case Opcode::kBranchIfNotA:
            case Opcode::kBranchIfNotError: {
                string label;
                *input >> label;
                labelUsages.push_back({ label, vec.size() });
                appendUint32(&vec, static_cast<uint32_t>(0));
                break;
            }

            case Opcode::kPopBranchIfError:
                appendUint32(&vec, input);
                appendUint32(&vec, input);
                appendUint32(&vec, input);
                break;

            case Opcode::kRecordBuilderBegin:
                appendUint16(&vec, input);
                appendUint16(&vec, input);
                break;

            case Opcode::kSystemCall: {
                string systemCallStr;
                *input >> systemCallStr;
                appendUint16(&vec, static_cast<uint16_t>(parseSystemCall(systemCallStr)));
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
