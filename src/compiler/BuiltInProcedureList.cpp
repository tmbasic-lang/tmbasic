#include "BuiltInProcedureList.h"

namespace compiler {

BuiltInProcedureList::BuiltInProcedureList() {
    auto number = boost::make_local_shared<TypeNode>(Kind::kNumber, Token{});
    auto string = boost::make_local_shared<TypeNode>(Kind::kString, Token{});
    auto date = boost::make_local_shared<TypeNode>(Kind::kDate, Token{});
    auto dateTime = boost::make_local_shared<TypeNode>(Kind::kDateTime, Token{});
    auto dateTimeOffset = boost::make_local_shared<TypeNode>(Kind::kDateTimeOffset, Token{});
    auto timeSpan = boost::make_local_shared<TypeNode>(Kind::kTimeSpan, Token{});
    auto timeZone = boost::make_local_shared<TypeNode>(Kind::kTimeZone, Token{});
    auto listOfString = boost::make_local_shared<TypeNode>(Kind::kList, Token{}, string);
    auto listOfNumber = boost::make_local_shared<TypeNode>(Kind::kList, Token{}, number);
    auto listGeneric = boost::make_local_shared<TypeNode>(Kind::kList, Token{});

    addFunction("Abs", { "x" }, { number }, number, vm::SystemCall::kAbs);
    addFunction("Acos", { "x" }, { number }, number, vm::SystemCall::kAcos);
    addFunction("Asin", { "x" }, { number }, number, vm::SystemCall::kAsin);
    addFunction("Atan", { "x" }, { number }, number, vm::SystemCall::kAtan);
    addFunction("Atan2", { "y", "x" }, { number, number }, number, vm::SystemCall::kAtan2);
    addFunction("AvailableLocales", {}, {}, listOfString, vm::SystemCall::kAvailableLocales);
    addFunction("AvailableTimeZones", {}, {}, listOfString, vm::SystemCall::kAvailableTimeZones);
    addFunction("Ceil", { "x" }, { number }, number, vm::SystemCall::kCeil);
    addFunction("Characters", { "this" }, { string }, listOfString, vm::SystemCall::kCharacters1);
    addFunction("Chr", { "input" }, { number }, string, vm::SystemCall::kChr);
    addFunction("CodePoints", { "input" }, { string }, listOfNumber, vm::SystemCall::kCodePoints);
    addFunction("CodeUnit", { "input" }, { string }, number, vm::SystemCall::kCodeUnit1);
    addFunction("CodeUnit", { "input", "index" }, { string, number }, number, vm::SystemCall::kCodeUnit2);
    addFunction("CodeUnits", { "input" }, { string }, listOfNumber, vm::SystemCall::kCodeUnits);
    addFunction("Concat", { "strings" }, { listOfString }, string, vm::SystemCall::kConcat1);
    addFunction("Concat", { "strings", "separator" }, { listOfString, string }, string, vm::SystemCall::kConcat2);
    addFunction("Cos", { "x" }, { number }, number, vm::SystemCall::kCos);
    addFunction(
        "DateFromParts", { "year", "month", "day" }, { number, number, number }, date, vm::SystemCall::kDateFromParts);
    addFunction(
        "DateTimeFromParts", { "year", "month", "day", "hour", "minute", "second", "millisecond" },
        { number, number, number, number, number, number, number }, dateTime, vm::SystemCall::kDateTimeFromParts);
    addFunction(
        "DateTimeOffsetFromParts", { "year", "month", "day", "hour", "minute", "second", "millisecond", "timeZone" },
        { number, number, number, number, number, number, number, timeZone }, dateTimeOffset,
        vm::SystemCall::kDateTimeOffsetFromParts);
    addFunction("Days", { "count" }, { number }, timeSpan, vm::SystemCall::kDays);
    addSub("DeleteFile", { "filePath" }, { string }, vm::SystemCall::kDeleteFile);
    addFunction("ErrorCode", {}, {}, number, vm::SystemCall::kErrorCode);
    addFunction("ErrorMessage", {}, {}, string, vm::SystemCall::kErrorMessage);
    addFunction("Exp", { "x" }, { number }, number, vm::SystemCall::kExp);
    addFunction("Floor", { "x" }, { number }, number, vm::SystemCall::kFloor);
    addFunction("Hours", { "count" }, { number }, timeSpan, vm::SystemCall::kHours);
    addFunction("Len", { "input" }, { listGeneric }, number, vm::SystemCall::kListLen);
    addFunction("Len", { "input" }, { string }, number, vm::SystemCall::kStringLen);
    addFunction("Log", { "x" }, { number }, number, vm::SystemCall::kLog);
    addFunction("Log10", { "x" }, { number }, number, vm::SystemCall::kLog10);
    addFunction("Milliseconds", { "count" }, { number }, timeSpan, vm::SystemCall::kMilliseconds);
    addFunction("Minutes", { "count" }, { number }, timeSpan, vm::SystemCall::kMinutes);
    addFunction("ReadFileLines", { "filePath" }, { string }, listOfString, vm::SystemCall::kReadFileLines);
    addFunction("ReadFileText", { "filePath" }, { string }, string, vm::SystemCall::kReadFileText);
    addFunction("Round", { "x" }, { number }, number, vm::SystemCall::kRound);
    addFunction("Seconds", { "count" }, { number }, timeSpan, vm::SystemCall::kSeconds);
    addFunction("Sin", { "x" }, { number }, number, vm::SystemCall::kSin);
    addFunction("Sqr", { "x" }, { number }, number, vm::SystemCall::kSqr);
    addFunction(
        "StringFromCodePoints", { "codePoints" }, { listOfNumber }, string, vm::SystemCall::kStringFromCodePoints);
    addFunction("StringFromCodeUnits", { "codeUnits" }, { listOfNumber }, string, vm::SystemCall::kStringFromCodeUnits);
    addFunction("Tan", { "x" }, { number }, number, vm::SystemCall::kTan);
    addFunction("TimeZoneFromName", { "name" }, { string }, timeZone, vm::SystemCall::kTimeZoneFromName);
    addFunction("TotalDays", { "timeSpan" }, { timeSpan }, number, vm::SystemCall::kTotalDays);
    addFunction("TotalHours", { "timeSpan" }, { timeSpan }, number, vm::SystemCall::kTotalHours);
    addFunction("TotalMilliseconds", { "timeSpan" }, { timeSpan }, number, vm::SystemCall::kTotalMilliseconds);
    addFunction("TotalMinutes", { "timeSpan" }, { timeSpan }, number, vm::SystemCall::kTotalMinutes);
    addFunction("TotalSeconds", { "timeSpan" }, { timeSpan }, number, vm::SystemCall::kTotalSeconds);
    addFunction("Trunc", { "x" }, { number }, number, vm::SystemCall::kTrunc);
    addSub("WriteFileLines", { "filePath", "lines" }, { string, listOfString }, vm::SystemCall::kWriteFileLines);
    addSub("WriteFileText", { "filePath", "text" }, { string, string }, vm::SystemCall::kWriteFileText);
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
    vm::SystemCall systemCall) {
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
    vm::SystemCall systemCall) {
    auto* node = addSub(std::move(name), parameterNames, parameterTypes, systemCall);
    node->returnType = std::move(returnType);
    return node;
}

}  // namespace compiler
