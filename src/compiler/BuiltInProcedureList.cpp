#include "BuiltInProcedureList.h"
#include "findBuiltInRecordType.h"

using vm::SystemCall;

namespace compiler {

BuiltInProcedureList::BuiltInProcedureList() {
    auto any = boost::make_local_shared<TypeNode>(Kind::kAny, Token{});
    auto boolean = boost::make_local_shared<TypeNode>(Kind::kBoolean, Token{});
    auto number = boost::make_local_shared<TypeNode>(Kind::kNumber, Token{});
    auto string = boost::make_local_shared<TypeNode>(Kind::kString, Token{});
    auto date = boost::make_local_shared<TypeNode>(Kind::kDate, Token{});
    auto dateTime = boost::make_local_shared<TypeNode>(Kind::kDateTime, Token{});
    auto dateTimeOffset = boost::make_local_shared<TypeNode>(Kind::kDateTimeOffset, Token{});
    auto timeSpan = boost::make_local_shared<TypeNode>(Kind::kTimeSpan, Token{});
    auto timeZone = boost::make_local_shared<TypeNode>(Kind::kTimeZone, Token{});
    auto listOfString = boost::make_local_shared<TypeNode>(Kind::kList, Token{}, string);
    auto listOfNumber = boost::make_local_shared<TypeNode>(Kind::kList, Token{}, number);
    auto listGeneric = boost::make_local_shared<TypeNode>(Kind::kList, Token{}, any);
    auto optionalGeneric = boost::make_local_shared<TypeNode>(Kind::kOptional, Token{}, any);
    auto generic1 = boost::make_local_shared<TypeNode>(Kind::kGeneric1, Token{});
    auto listOfGeneric1 = boost::make_local_shared<TypeNode>(Kind::kList, Token{}, generic1);
    auto form = boost::make_local_shared<TypeNode>(Kind::kForm, Token{});
    auto control = boost::make_local_shared<TypeNode>(Kind::kControl, Token{});
    auto rectangle = boost::make_local_shared<TypeNode>(Kind::kRecord, Token{}, "Rectangle");
    if (!findBuiltInRecordType("rectangle", &rectangle->fields)) {
        throw std::runtime_error("Built-in record type Rectangle not found");
    }

    addFunction("Abs", { "x" }, { number }, number, SystemCall::kAbs);
    addFunction("Acos", { "x" }, { number }, number, SystemCall::kAcos);
    addSub("AddControlToForm", { "form", "control" }, { form, control }, SystemCall::kAddControlToForm);
    addFunction("Asin", { "x" }, { number }, number, SystemCall::kAsin);
    addFunction("Atan", { "x" }, { number }, number, SystemCall::kAtan);
    addFunction("Atan2", { "y", "x" }, { number, number }, number, SystemCall::kAtan2);
    addFunction("Ceil", { "x" }, { number }, number, SystemCall::kCeil);
    addFunction("Characters", { "this" }, { string }, listOfString, SystemCall::kCharacters);
    addFunction("Chr", { "input" }, { number }, string, SystemCall::kChr);
    addFunction("CodePoints", { "input" }, { string }, listOfNumber, SystemCall::kCodePoints);
    addFunction("CodeUnit", { "input" }, { string }, number, SystemCall::kCodeUnit1);
    addFunction("CodeUnit", { "input", "index" }, { string, number }, number, SystemCall::kCodeUnit2);
    addFunction("CodeUnits", { "input" }, { string }, listOfNumber, SystemCall::kCodeUnits);
    addFunction("Concat", { "strings" }, { listOfString }, string, SystemCall::kConcat1);
    addFunction("Concat", { "strings", "separator" }, { listOfString, string }, string, SystemCall::kConcat2);
    addFunction("ControlBounds", { "control" }, { control }, rectangle, SystemCall::kControlBounds);
    addFunction("ControlText", { "control" }, { control }, string, SystemCall::kControlText);
    addFunction("Cos", { "x" }, { number }, number, SystemCall::kCos);
    addSub("CreateDirectory", { "path" }, { string }, SystemCall::kCreateDirectory);
    addFunction(
        "DateFromParts", { "year", "month", "day" }, { number, number, number }, date, SystemCall::kDateFromParts);
    addFunction(
        "DateTimeFromParts", { "year", "month", "day", "hour", "minute", "second", "millisecond" },
        { number, number, number, number, number, number, number }, dateTime, SystemCall::kDateTimeFromParts);
    addFunction(
        "DateTimeOffsetFromParts", { "year", "month", "day", "hour", "minute", "second", "millisecond", "utcOffset" },
        { number, number, number, number, number, number, number, timeSpan }, dateTimeOffset,
        SystemCall::kDateTimeOffsetFromParts);
    addFunction("Days", { "count" }, { number }, timeSpan, SystemCall::kDays);
    addSub("DeleteDirectory", { "path" }, { string }, SystemCall::kDeleteDirectory1);
    addSub("DeleteDirectory", { "path", "recursive" }, { string, boolean }, SystemCall::kDeleteDirectory2);
    addSub("DeleteFile", { "filePath" }, { string }, SystemCall::kDeleteFile);
    addFunction("ErrorCode", {}, {}, number, SystemCall::kErrorCode);
    addFunction("ErrorMessage", {}, {}, string, SystemCall::kErrorMessage);
    addFunction("Exp", { "x" }, { number }, number, SystemCall::kExp);
    addFunction("FileExists", { "filePath" }, { string }, boolean, SystemCall::kFileExists);
    addFunction("First", { "list" }, { listGeneric }, generic1, SystemCall::kListFirst);
    addFunction("Floor", { "x" }, { number }, number, SystemCall::kFloor);
    addFunction("FormTitle", { "form" }, { form }, string, SystemCall::kFormTitle);
    addFunction("HasValue", { "this" }, { optionalGeneric }, boolean, SystemCall::kHasValue);
    addFunction("Hours", { "count" }, { number }, timeSpan, SystemCall::kHours);
    addFunction("Last", { "list" }, { listGeneric }, generic1, SystemCall::kListLast);
    addFunction("Len", { "input" }, { listGeneric }, number, SystemCall::kListLen);
    addFunction("Len", { "input" }, { string }, number, SystemCall::kStringLen);
    addFunction("ListDirectories", { "path" }, { string }, listOfString, SystemCall::kListDirectories);
    addFunction("ListFiles", { "path" }, { string }, listOfString, SystemCall::kListFiles);
    addFunction("ListFill", { "value", "count" }, { any, number }, listOfGeneric1, SystemCall::kListFillO);
    addFunction("Log", { "x" }, { number }, number, SystemCall::kLog);
    addFunction("Log10", { "x" }, { number }, number, SystemCall::kLog10);
    addFunction(
        "Mid", { "list", "start", "count" }, { listGeneric, number, number }, listOfGeneric1, SystemCall::kListMid);
    addFunction("Milliseconds", { "count" }, { number }, timeSpan, SystemCall::kMilliseconds);
    addFunction("Minutes", { "count" }, { number }, timeSpan, SystemCall::kMinutes);
    addFunction("NewForm", {}, {}, form, SystemCall::kNewForm);
    addFunction("NewLabel", {}, {}, control, SystemCall::kNewLabel);
    addFunction("NewLine", {}, {}, string, SystemCall::kNewLine);
    addFunction("ParseNumber", { "text" }, { string }, number, SystemCall::kParseNumber);
    addFunction("PathCombine", { "parts" }, { listOfString }, string, SystemCall::kPathCombine);
    addFunction("PathDirectoryName", { "path" }, { string }, string, SystemCall::kPathDirectoryName);
    addFunction("PathExtension", { "path" }, { string }, string, SystemCall::kPathExtension);
    addFunction("PathFileName", { "path" }, { string }, string, SystemCall::kPathFileName);
    addFunction(
        "PathFileNameWithoutExtension", { "path" }, { string }, string, SystemCall::kPathFileNameWithoutExtension);
    addFunction("PathSeparator", {}, {}, string, SystemCall::kPathSeparator);
    addFunction("ReadFileBytes", { "filePath" }, { string }, listOfNumber, SystemCall::kReadFileBytes);
    addFunction("ReadFileLines", { "filePath" }, { string }, listOfString, SystemCall::kReadFileLines);
    addFunction("ReadFileText", { "filePath" }, { string }, string, SystemCall::kReadFileText);
    addFunction("Round", { "x" }, { number }, number, SystemCall::kRound);
    addSub("RunForm", { "form" }, { form }, SystemCall::kRunForm);
    addFunction("Seconds", { "count" }, { number }, timeSpan, SystemCall::kSeconds);
    addSub("SetControlBounds", { "control", "bounds" }, { control, rectangle }, SystemCall::kSetControlBounds1);
    addSub(
        "SetControlBounds", { "control", "left", "top", "width", "height" },
        { control, number, number, number, number }, SystemCall::kSetControlBounds2);
    addSub("SetControlText", { "control", "text" }, { control, string }, SystemCall::kSetControlText);
    addSub("SetFormTitle", { "form", "title" }, { form, string }, SystemCall::kSetFormTitle);
    addFunction("Sin", { "x" }, { number }, number, SystemCall::kSin);
    addFunction("Skip", { "list", "count" }, { listGeneric, number }, listOfGeneric1, SystemCall::kListSkip);
    addFunction("Sqr", { "x" }, { number }, number, SystemCall::kSqr);
    addFunction("StringFromCodePoints", { "codePoints" }, { listOfNumber }, string, SystemCall::kStringFromCodePoints);
    addFunction("StringFromCodeUnits", { "codeUnits" }, { listOfNumber }, string, SystemCall::kStringFromCodeUnits);
    addFunction("Take", { "list", "count" }, { listGeneric, number }, listOfGeneric1, SystemCall::kListTake);
    addFunction("Tan", { "x" }, { number }, number, SystemCall::kTan);
    addFunction("TimeZoneFromName", { "name" }, { string }, timeZone, SystemCall::kTimeZoneFromName);
    addFunction("TotalDays", { "timeSpan" }, { timeSpan }, number, SystemCall::kTotalDays);
    addFunction("TotalHours", { "timeSpan" }, { timeSpan }, number, SystemCall::kTotalHours);
    addFunction("TotalMilliseconds", { "timeSpan" }, { timeSpan }, number, SystemCall::kTotalMilliseconds);
    addFunction("TotalMinutes", { "timeSpan" }, { timeSpan }, number, SystemCall::kTotalMinutes);
    addFunction("TotalSeconds", { "timeSpan" }, { timeSpan }, number, SystemCall::kTotalSeconds);
    addFunction("Trunc", { "x" }, { number }, number, SystemCall::kTrunc);
    addFunction("Value", { "this" }, { optionalGeneric }, generic1, SystemCall::kValue);
    addSub("WriteFileBytes", { "filePath", "bytes" }, { string, listOfNumber }, SystemCall::kWriteFileBytes);
    addSub("WriteFileLines", { "filePath", "lines" }, { string, listOfString }, SystemCall::kWriteFileLines);
    addSub("WriteFileText", { "filePath", "text" }, { string, string }, SystemCall::kWriteFileText);
}

const std::vector<std::unique_ptr<ProcedureNode>>& BuiltInProcedureList::get(const std::string& name) const {
    auto lowercaseName = boost::to_lower_copy(name);
    auto result = map.find(lowercaseName);
    return result == map.end() ? _empty : *result->second;
}

std::vector<std::unique_ptr<ProcedureNode>>* BuiltInProcedureList::findOrCreateVector(
    const std::string& lowercaseName) {
    auto it = map.find(lowercaseName);
    if (it != map.end()) {
        return it->second.get();
    }

    auto procedureNode = std::make_unique<std::vector<std::unique_ptr<ProcedureNode>>>();
    auto* procedureNodePtr = procedureNode.get();
    map.insert(std::pair(lowercaseName, std::move(procedureNode)));
    return procedureNodePtr;
}

ProcedureNode* BuiltInProcedureList::addSub(
    std::string name,
    std::initializer_list<std::string> parameterNames,
    std::initializer_list<boost::local_shared_ptr<TypeNode>> parameterTypes,
    SystemCall systemCall) {
    auto lowercaseName = boost::to_lower_copy(name);
    assert(parameterNames.size() == parameterTypes.size());
    std::vector<std::unique_ptr<ParameterNode>> parameterNodes{};
    const auto* parameterNameIter = parameterNames.begin();
    const auto* parameterTypesIter = parameterTypes.begin();
    for (size_t i = 0; i < parameterNames.size(); i++) {
        parameterNodes.push_back(std::make_unique<ParameterNode>(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            *parameterNameIter++, *parameterTypesIter++, Token{}));
    }
    auto procedure = std::make_unique<ProcedureNode>(std::move(name), std::move(parameterNodes), nullptr, Token{});
    procedure->systemCall = systemCall;
    auto* procedurePtr = procedure.get();
    auto* vec = findOrCreateVector(lowercaseName);
    vec->push_back(std::move(procedure));
    return procedurePtr;
}

ProcedureNode* BuiltInProcedureList::addFunction(
    std::string name,
    std::initializer_list<std::string> parameterNames,
    std::initializer_list<boost::local_shared_ptr<TypeNode>> parameterTypes,
    boost::local_shared_ptr<TypeNode> returnType,
    SystemCall systemCall) {
    auto* node = addSub(std::move(name), parameterNames, parameterTypes, systemCall);
    node->returnType = std::move(returnType);
    return node;
}

}  // namespace compiler
