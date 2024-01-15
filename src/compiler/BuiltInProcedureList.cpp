#include "BuiltInProcedureList.h"
#include "BuiltInRecordTypesList.h"

using shared::SystemCall;

namespace compiler {

static boost::local_shared_ptr<TypeNode> getBuiltInRecordType(const std::string& name) {
    auto typeNode = boost::make_local_shared<TypeNode>(Kind::kRecord, Token{}, name);
    auto lowercaseName = boost::to_lower_copy(name);
    auto found = findBuiltInRecordType(lowercaseName, &typeNode->fields);
    if (!found) {
        throw std::runtime_error(fmt::format("Internal error. Built-in record type \"{}\" not found.", name));
    }
    return typeNode;
}

BuiltInProcedureList::BuiltInProcedureList() {
    auto any = boost::make_local_shared<TypeNode>(Kind::kAny, Token{});
    auto boolean = boost::make_local_shared<TypeNode>(Kind::kBoolean, Token{});
    auto date = boost::make_local_shared<TypeNode>(Kind::kDate, Token{});
    auto dateTime = boost::make_local_shared<TypeNode>(Kind::kDateTime, Token{});
    auto dateTimeOffset = boost::make_local_shared<TypeNode>(Kind::kDateTimeOffset, Token{});
    auto number = boost::make_local_shared<TypeNode>(Kind::kNumber, Token{});
    auto string = boost::make_local_shared<TypeNode>(Kind::kString, Token{});
    auto timeSpan = boost::make_local_shared<TypeNode>(Kind::kTimeSpan, Token{});
    auto timeZone = boost::make_local_shared<TypeNode>(Kind::kTimeZone, Token{});

    auto generic1 = boost::make_local_shared<TypeNode>(Kind::kGeneric1, Token{});
    auto generic2 = boost::make_local_shared<TypeNode>(Kind::kGeneric2, Token{});

    auto listOfAny = boost::make_local_shared<TypeNode>(Kind::kList, Token{}, any);
    auto listOfGeneric1 = boost::make_local_shared<TypeNode>(Kind::kList, Token{}, generic1);
    auto listOfNumber = boost::make_local_shared<TypeNode>(Kind::kList, Token{}, number);
    auto listOfString = boost::make_local_shared<TypeNode>(Kind::kList, Token{}, string);
    auto mapFromAnyToAny = boost::make_local_shared<TypeNode>(Kind::kMap, Token{}, any, any);
    auto optionalAny = boost::make_local_shared<TypeNode>(Kind::kOptional, Token{}, any);
    auto optionalGeneric2 = boost::make_local_shared<TypeNode>(Kind::kOptional, Token{}, generic2);
    auto setOfAny = boost::make_local_shared<TypeNode>(Kind::kSet, Token{}, any);

    auto form = boost::make_local_shared<TypeNode>(Kind::kForm, Token{});
    auto control = boost::make_local_shared<TypeNode>(Kind::kControl, Token{});

    auto color = getBuiltInRecordType("Color");
    auto rectangle = getBuiltInRecordType("Rectangle");

    addFunction("Abs", { "x" }, { number }, number, SystemCall::kAbs);
    addFunction("Acos", { "x" }, { number }, number, SystemCall::kAcos);
    addFunction("AddDays", { "time", "count" }, { date, number }, date, SystemCall::kDateTimeAddDays);
    addFunction("AddDays", { "time", "count" }, { dateTime, number }, dateTime, SystemCall::kDateTimeAddDays);
    addFunction(
        "AddDays", { "time", "count" }, { dateTimeOffset, number }, dateTimeOffset, SystemCall::kDateTimeOffsetAddDays);
    addFunction("AddHours", { "time", "count" }, { dateTime, number }, dateTime, SystemCall::kDateTimeAddHours);
    addFunction(
        "AddHours", { "time", "count" }, { dateTimeOffset, number }, dateTimeOffset,
        SystemCall::kDateTimeOffsetAddHours);
    addFunction(
        "AddMilliseconds", { "time", "count" }, { dateTime, number }, dateTime, SystemCall::kDateTimeAddMilliseconds);
    addFunction(
        "AddMilliseconds", { "time", "count" }, { dateTimeOffset, number }, dateTimeOffset,
        SystemCall::kDateTimeOffsetAddMilliseconds);
    addFunction("AddMinutes", { "time", "count" }, { dateTime, number }, dateTime, SystemCall::kDateTimeAddMinutes);
    addFunction(
        "AddMinutes", { "time", "count" }, { dateTimeOffset, number }, dateTimeOffset,
        SystemCall::kDateTimeOffsetAddMinutes);
    addFunction("AddMonths", { "time", "count" }, { date, number }, date, SystemCall::kDateTimeAddMonths);
    addFunction("AddMonths", { "time", "count" }, { dateTime, number }, dateTime, SystemCall::kDateTimeAddMonths);
    addFunction(
        "AddMonths", { "time", "count" }, { dateTimeOffset, number }, dateTimeOffset,
        SystemCall::kDateTimeOffsetAddMonths);
    addFunction("AddSeconds", { "time", "count" }, { dateTime, number }, dateTime, SystemCall::kDateTimeAddSeconds);
    addFunction(
        "AddSeconds", { "time", "count" }, { dateTimeOffset, number }, dateTimeOffset,
        SystemCall::kDateTimeOffsetAddSeconds);
    addFunction("AddYears", { "time", "count" }, { date, number }, date, SystemCall::kDateTimeAddYears);
    addFunction("AddYears", { "time", "count" }, { dateTime, number }, dateTime, SystemCall::kDateTimeAddYears);
    addFunction(
        "AddYears", { "time", "count" }, { dateTimeOffset, number }, dateTimeOffset,
        SystemCall::kDateTimeOffsetAddYears);
    addFunction("Asin", { "x" }, { number }, number, SystemCall::kAsin);
    addFunction("Atan", { "x" }, { number }, number, SystemCall::kAtan);
    addFunction("Atan2", { "y", "x" }, { number, number }, number, SystemCall::kAtan2);
    addFunction("Ceil", { "x" }, { number }, number, SystemCall::kCeil);
    addFunction("Characters", { "this" }, { string }, listOfString, SystemCall::kCharacters);
    addFunction("Chr", { "input" }, { number }, string, SystemCall::kChr);
    addFunction("CodePoints", { "input" }, { string }, listOfNumber, SystemCall::kCodePoints);
    addSub("AddControlToForm", { "form", "control" }, { form, control }, SystemCall::kAddControlToForm);
    addFunction("CodeUnit", { "input" }, { string }, number, SystemCall::kCodeUnit1);
    addFunction("CodeUnit", { "input", "index" }, { string, number }, number, SystemCall::kCodeUnit2);
    addFunction("CodeUnits", { "input" }, { string }, listOfNumber, SystemCall::kCodeUnits);
    addFunction("Concat", { "strings" }, { listOfString }, string, SystemCall::kConcat1);
    addFunction("Concat", { "strings", "separator" }, { listOfString, string }, string, SystemCall::kConcat2);
    addFunction("Contains", { "set", "key" }, { setOfAny, generic1 }, boolean, SystemCall::kSetContains);
    addFunction("ContainsKey", { "map", "key" }, { mapFromAnyToAny, generic1 }, boolean, SystemCall::kMapContainsKey);
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
    addFunction("Day", { "input" }, { date }, number, SystemCall::kDateTimeOffsetDay);
    addFunction("Day", { "input" }, { dateTime }, number, SystemCall::kDateTimeOffsetDay);
    addFunction("Day", { "input" }, { dateTimeOffset }, number, SystemCall::kDateTimeOffsetDay);
    addFunction("Days", { "count" }, { number }, timeSpan, SystemCall::kDays);
    addSub("DeleteDirectory", { "path" }, { string }, SystemCall::kDeleteDirectory1);
    addSub("DeleteDirectory", { "path", "recursive" }, { string, boolean }, SystemCall::kDeleteDirectory2);
    addSub("DeleteFile", { "filePath" }, { string }, SystemCall::kDeleteFile);
    addSub("EnterFullscreen", {}, {}, SystemCall::kEnterFullscreen);
    addFunction("ErrorCode", {}, {}, number, SystemCall::kErrorCode);
    addFunction("ErrorMessage", {}, {}, string, SystemCall::kErrorMessage);
    addFunction("Exp", { "x" }, { number }, number, SystemCall::kExp);
    addFunction("FileExists", { "filePath" }, { string }, boolean, SystemCall::kFileExists);
    addFunction("Find", { "map", "key" }, { mapFromAnyToAny, generic1 }, optionalGeneric2, SystemCall::kMapFind);
    addFunction("First", { "list" }, { listOfAny }, generic1, SystemCall::kListFirst);
    addFunction("Floor", { "x" }, { number }, number, SystemCall::kFloor);
    addFunction("FormTitle", { "form" }, { form }, string, SystemCall::kFormTitle);
    addFunction("HasValue", { "this" }, { optionalAny }, boolean, SystemCall::kHasValue);
    addFunction("Hour", { "input" }, { dateTime }, number, SystemCall::kDateTimeOffsetHour);
    addFunction("Hour", { "input" }, { dateTimeOffset }, number, SystemCall::kDateTimeOffsetHour);
    addFunction("Hours", { "count" }, { number }, timeSpan, SystemCall::kHours);
    addFunction("IsDigit", { "input" }, { string }, boolean, SystemCall::kIsDigit);
    addFunction("Last", { "list" }, { listOfAny }, generic1, SystemCall::kListLast);
    addFunction("Len", { "input" }, { listOfAny }, number, SystemCall::kListLen);
    addFunction("Len", { "input" }, { mapFromAnyToAny }, number, SystemCall::kMapLen);
    addFunction("Len", { "input" }, { setOfAny }, number, SystemCall::kSetLen);
    addFunction("Len", { "input" }, { string }, number, SystemCall::kStringLen);
    addFunction("ListDirectories", { "path" }, { string }, listOfString, SystemCall::kListDirectories);
    addFunction("ListFiles", { "path" }, { string }, listOfString, SystemCall::kListFiles);
    addFunction("ListFill", { "value", "count" }, { any, number }, listOfGeneric1, SystemCall::kListFillO);
    addFunction("Log", { "x" }, { number }, number, SystemCall::kLog);
    addFunction("Log10", { "x" }, { number }, number, SystemCall::kLog10);
    addFunction(
        "Mid", { "list", "start", "count" }, { listOfAny, number, number }, listOfGeneric1, SystemCall::kListMid);
    addFunction("Millisecond", { "input" }, { dateTime }, number, SystemCall::kDateTimeOffsetMillisecond);
    addFunction("Millisecond", { "input" }, { dateTimeOffset }, number, SystemCall::kDateTimeOffsetMillisecond);
    addFunction("Milliseconds", { "count" }, { number }, timeSpan, SystemCall::kMilliseconds);
    addFunction("Minute", { "input" }, { dateTime }, number, SystemCall::kDateTimeOffsetMinute);
    addFunction("Minute", { "input" }, { dateTimeOffset }, number, SystemCall::kDateTimeOffsetMinute);
    addFunction("Minutes", { "count" }, { number }, timeSpan, SystemCall::kMinutes);
    addFunction("Month", { "input" }, { date }, number, SystemCall::kDateTimeOffsetMonth);
    addFunction("Month", { "input" }, { dateTime }, number, SystemCall::kDateTimeOffsetMonth);
    addFunction("Month", { "input" }, { dateTimeOffset }, number, SystemCall::kDateTimeOffsetMonth);
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
    addFunction("RemoveAt", { "list", "index" }, { listOfAny, number }, listOfGeneric1, SystemCall::kListRemoveAt1);
    addFunction(
        "RemoveAt", { "list", "indices" }, { listOfAny, listOfNumber }, listOfGeneric1, SystemCall::kListRemoveAt2);
    addFunction(
        "Replace", { "haystack", "needle", "replacement" }, { string, string, string }, string,
        SystemCall::kStringReplace);
    addFunction("Rgb", { "red", "green", "blue" }, { number, number, number }, color, SystemCall::kRgb);
    addFunction("Round", { "x" }, { number }, number, SystemCall::kRound);
    addSub("RunForm", { "form" }, { form }, SystemCall::kRunForm);
    addFunction("Second", { "input" }, { dateTime }, number, SystemCall::kDateTimeOffsetSecond);
    addFunction("Second", { "input" }, { dateTimeOffset }, number, SystemCall::kDateTimeOffsetSecond);
    addFunction("Seconds", { "count" }, { number }, timeSpan, SystemCall::kSeconds);
    addSub("SetControlBounds", { "control", "bounds" }, { control, rectangle }, SystemCall::kSetControlBounds1);
    addSub(
        "SetControlBounds", { "control", "left", "top", "width", "height" },
        { control, number, number, number, number }, SystemCall::kSetControlBounds2);
    addSub("SetControlText", { "control", "text" }, { control, string }, SystemCall::kSetControlText);
    addSub("SetFormTitle", { "form", "title" }, { form, string }, SystemCall::kSetFormTitle);
    addSub("SetFullscreenBuffering", { "enable" }, { boolean }, SystemCall::kSetFullscreenBuffering);
    addFunction("Sin", { "x" }, { number }, number, SystemCall::kSin);
    addFunction("Skip", { "list", "count" }, { listOfAny, number }, listOfGeneric1, SystemCall::kListSkip);
    addSub("Sleep", { "delay" }, { timeSpan }, SystemCall::kSleep);
    addFunction("Split", { "input", "separator" }, { string, string }, listOfString, SystemCall::kStringSplit);
    addFunction("Sqr", { "x" }, { number }, number, SystemCall::kSqr);
    addFunction("StringFromCodePoints", { "codePoints" }, { listOfNumber }, string, SystemCall::kStringFromCodePoints);
    addFunction("StringFromCodeUnits", { "codeUnits" }, { listOfNumber }, string, SystemCall::kStringFromCodeUnits);
    addFunction("Take", { "list", "count" }, { listOfAny, number }, listOfGeneric1, SystemCall::kListTake);
    addFunction("Tan", { "x" }, { number }, number, SystemCall::kTan);
    addFunction("TimeZoneFromName", { "name" }, { string }, timeZone, SystemCall::kTimeZoneFromName);
    addFunction("TotalDays", { "timeSpan" }, { timeSpan }, number, SystemCall::kTotalDays);
    addFunction("TotalHours", { "timeSpan" }, { timeSpan }, number, SystemCall::kTotalHours);
    addFunction("TotalMilliseconds", { "timeSpan" }, { timeSpan }, number, SystemCall::kTotalMilliseconds);
    addFunction("TotalMinutes", { "timeSpan" }, { timeSpan }, number, SystemCall::kTotalMinutes);
    addFunction("TotalSeconds", { "timeSpan" }, { timeSpan }, number, SystemCall::kTotalSeconds);
    addFunction("Trunc", { "x" }, { number }, number, SystemCall::kTrunc);
    addSub("UpdateScreen", {}, {}, SystemCall::kUpdateScreen);
    addFunction("Value", { "this" }, { optionalAny }, generic1, SystemCall::kValue);
    addFunction("Values", { "set" }, { setOfAny }, listOfGeneric1, SystemCall::kSetValues);
    addSub("WriteFileBytes", { "filePath", "bytes" }, { string, listOfNumber }, SystemCall::kWriteFileBytes);
    addSub("WriteFileLines", { "filePath", "lines" }, { string, listOfString }, SystemCall::kWriteFileLines);
    addSub("WriteFileText", { "filePath", "text" }, { string, string }, SystemCall::kWriteFileText);
    addFunction("Year", { "input" }, { date }, number, SystemCall::kDateTimeOffsetYear);
    addFunction("Year", { "input" }, { dateTime }, number, SystemCall::kDateTimeOffsetYear);
    addFunction("Year", { "input" }, { dateTimeOffset }, number, SystemCall::kDateTimeOffsetYear);
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
