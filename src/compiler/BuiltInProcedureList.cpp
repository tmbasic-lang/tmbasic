#include "BuiltInProcedureList.h"

namespace compiler {

std::vector<std::unique_ptr<ProcedureNode>>* BuiltInProcedureList::findOrCreateVector(
    const std::string& lowercaseName) {
    auto it = _map.find(lowercaseName);
    if (it != _map.end()) {
        return it->second.get();
    }

    auto procedureNode = std::make_unique<std::vector<std::unique_ptr<ProcedureNode>>>();
    auto procedureNodePtr = procedureNode.get();
    _map.insert(std::pair(lowercaseName, std::move(procedureNode)));
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
    auto* parameterNameIter = parameterNames.begin();
    auto* parameterTypesIter = parameterTypes.begin();
    for (size_t i = 0; i < parameterNames.size(); i++) {
        parameterNodes.push_back(std::make_unique<ParameterNode>(
            std::move(*parameterNameIter++), std::move(*parameterTypesIter++), Token{}));
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
    auto* node = addSub(name, parameterNames, parameterTypes, systemCall);
    node->returnType = std::move(returnType);
    return node;
}

BuiltInProcedureList::BuiltInProcedureList() {
    auto number = boost::make_local_shared<TypeNode>(Kind::kNumber, Token{});
    auto string = boost::make_local_shared<TypeNode>(Kind::kString, Token{});
    auto listOfString = boost::make_local_shared<TypeNode>(Kind::kList, Token{}, string);

    addFunction("Chr", { "input" }, { number }, string, vm::SystemCall::kChr);
}

const std::vector<std::unique_ptr<ProcedureNode>>& BuiltInProcedureList::get(const std::string& name) {
    auto lowercaseName = boost::to_lower_copy(name);
    auto result = _map.find(lowercaseName);
    return result == _map.end() ? _empty : *result->second;
}

}  // namespace compiler
