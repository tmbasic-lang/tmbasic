#include "assemble.h"
#include "shared/vm/Opcode.h"

using std::array;
using std::istream;
using std::regex;
using std::regex_search;
using std::smatch;
using std::string;
using std::unordered_map;
using std::vector;
using vm::Opcode;

static Opcode parseOpcode(string s) {
    // generate this map using:
    // grep , src/shared/vm/Opcode.h | sed s/,//g | awk '{ print "{\"" $1 "\",Opcode::" $1 "}," }' | sed 's/"k/"/g'
    static const std::unordered_map<string, Opcode> map = {
        { "Exit", Opcode::kExit },
        { "LoadConstantA", Opcode::kLoadConstantA },
        { "LoadConstantB", Opcode::kLoadConstantB },
        { "LoadConstantStringX", Opcode::kLoadConstantStringX },
        { "LoadConstantStringY", Opcode::kLoadConstantStringY },
        { "StoreA", Opcode::kStoreA },
        { "StoreB", Opcode::kStoreB },
        { "StoreX", Opcode::kStoreX },
        { "StoreY", Opcode::kStoreY },
        { "LoadA", Opcode::kLoadA },
        { "LoadB", Opcode::kLoadB },
        { "LoadX", Opcode::kLoadX },
        { "LoadY", Opcode::kLoadY },
        { "PushA", Opcode::kPushA },
        { "PushB", Opcode::kPushB },
        { "PushX", Opcode::kPushX },
        { "PushY", Opcode::kPushY },
        { "PopA", Opcode::kPopA },
        { "PopB", Opcode::kPopB },
        { "PopX", Opcode::kPopX },
        { "PopY", Opcode::kPopY },
        { "PopValues", Opcode::kPopValues },
        { "PopObjects", Opcode::kPopObjects },
        { "ClearX", Opcode::kClearX },
        { "ClearY", Opcode::kClearY },
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
        { "ValueListCount", Opcode::kValueListCount },
        { "ValueListInsert", Opcode::kValueListInsert },
        { "ValueListRemove", Opcode::kValueListRemove },
        { "ObjectListBuilderBegin", Opcode::kObjectListBuilderBegin },
        { "ObjectListBuilderAddX", Opcode::kObjectListBuilderAddX },
        { "ObjectListBuilderEnd", Opcode::kObjectListBuilderEnd },
        { "ObjectListGet", Opcode::kObjectListGet },
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
        { "StringMid", Opcode::kStringMid },
        { "StringIndexOf", Opcode::kStringIndexOf },
        { "StringChr", Opcode::kStringChr },
        { "StringAsc", Opcode::kStringAsc },
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

static void appendUint16(vector<uint8_t>* vec, uint16_t value) {
    auto bytes = array<uint8_t, 2>();
    memcpy(bytes.data(), &value, 2);
    for (auto b : bytes) {
        vec->push_back(b);
    }
}

static void appendUint16(vector<uint8_t>* vec, istream* input) {
    uint16_t value = 0;
    *input >> value;
    appendUint16(vec, value);
}

static void appendUint32(vector<uint8_t>* vec, istream* input) {
    uint32_t value = 0;
    *input >> value;
    auto bytes = array<uint8_t, 4>();
    memcpy(bytes.data(), &value, 4);
    for (auto b : bytes) {
        vec->push_back(b);
    }
}

static void appendLengthTaggedString(vector<uint8_t>* vec, istream* input) {
    string quoted;
    std::getline(*input, quoted);

    // strip leading and trailing whitespace and quotes
    smatch match;
    auto regex_success = regex_search(quoted, match, regex("^\\s*\"([^\"]*)\"\\s*$"));
    assert(regex_success);
    auto unquoted = match[1].str();

    appendUint16(vec, static_cast<uint16_t>(unquoted.size()));
    for (auto ch : unquoted) {
        vec->push_back(static_cast<uint8_t>(ch));
    }
}

vector<uint8_t> assemble(istream* input) {
    auto vec = vector<uint8_t>();

    while (!input->eof()) {
        string opcodeStr;
        *input >> opcodeStr;
        if (opcodeStr == "") {
            return vec;
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
                appendLengthTaggedString(&vec, input);
                break;

            case Opcode::kStoreA:
            case Opcode::kStoreB:
            case Opcode::kStoreX:
            case Opcode::kStoreY:
            case Opcode::kLoadA:
            case Opcode::kLoadB:
            case Opcode::kLoadX:
            case Opcode::kLoadY:
            case Opcode::kPopValues:
            case Opcode::kPopObjects:
            case Opcode::kRecordBuilderStoreA:
            case Opcode::kRecordBuilderStoreX:
            case Opcode::kRecordLoadA:
            case Opcode::kRecordLoadX:
            case Opcode::kRecordStoreA:
            case Opcode::kRecordStoreY:
                appendUint16(&vec, input);
                break;

            case Opcode::kJump:
            case Opcode::kBranchIfA:
            case Opcode::kBranchIfNotA:
            case Opcode::kBranchIfNotError:
                appendUint32(&vec, input);
                break;

            case Opcode::kPopBranchIfError:
                appendUint32(&vec, input);
                appendUint32(&vec, input);
                appendUint32(&vec, input);
                break;

            case Opcode::kRecordBuilderBegin:
                appendUint16(&vec, input);
                appendUint16(&vec, input);
                break;

            default:
                break;
        }
    }

    return vec;
}
