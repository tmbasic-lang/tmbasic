#include "compiler/checkRecursiveTypes.h"
#include "compiler/BuiltInRecordTypesList.h"
#include "compiler/CompilerException.h"
#include "shared/strings.h"

namespace compiler {

namespace {

class State {
   public:
    // Either typeNode or name will be provided, but not both.
    TypeNode* typeNode;
    std::optional<std::string> name;
    immer::flex_vector<std::string> path;
    CompiledProgram* compiledProgram;

    State(TypeNode* typeNode, immer::flex_vector<std::string> path, CompiledProgram* compiledProgram)
        : typeNode(typeNode), path(std::move(path)), compiledProgram(compiledProgram) {}

    State(std::string name, immer::flex_vector<std::string> path, CompiledProgram* compiledProgram)
        : typeNode(nullptr), name(std::move(name)), path(std::move(path)), compiledProgram(compiledProgram) {}

    State next(TypeNode* typeNode, immer::flex_vector<std::string> path) const {
        return State{ typeNode, std::move(path), compiledProgram };
    }

    State next(std::string name, immer::flex_vector<std::string> path) const {
        return State{ std::move(name), std::move(path), compiledProgram };
    }

    ~State() = default;

    // Non-copyable
    State(const State&) = delete;
    State& operator=(const State&) = delete;
    State(State&&) = delete;
    State& operator=(State&&) = delete;
};

void reportRecursiveTypeError(
    const std::string& typeName,
    const immer::flex_vector<std::string>& path,
    const Token& token) {
    std::ostringstream ss{};
    ss << "The type \"" << typeName << "\" contains a recursive type reference which is not allowed. The cycle is: ";

    auto isFirst = true;
    for (const auto& p : path) {
        if (!isFirst) {
            ss << " -> ";
        }
        ss << p;
        isFirst = false;
    }

    throw CompilerException(CompilerErrorCode::kRecursiveRecordType, ss.str(), token);
}

void walkType(const State& state) {
    if (state.typeNode != nullptr) {
        // All types in TMBASIC have names, so let's construct the name as a string and use that to check for cycles.
        // Because names are case insensitive and this is prior to type binding, we have to handle case insensitivity
        // ourselves here. We will use the lowercase names.
        const auto& typeNode = *state.typeNode;
        const auto typeName = typeNode.toString();
        const auto typeNameLowercase = shared::to_lower_copy(typeName);
        const auto newPath = state.path.push_back(typeNameLowercase);

        // Check if the type is already in the path. If it is, this is a cycle.
        if (std::find(state.path.begin(), state.path.end(), typeNameLowercase) != state.path.end()) {
            reportRecursiveTypeError(typeName, newPath, typeNode.token);
        }

        switch (typeNode.kind) {
            case Kind::kRecord:
                if (typeNode.recordName.has_value()) {
                    // Named record.
                    const auto& recordName = *typeNode.recordName;
                    walkType(state.next(recordName, state.path));
                } else {
                    // Anonymous record
                    for (const auto& field : typeNode.fields) {
                        walkType(state.next(field->type.get(), newPath));
                    }
                }
                break;

            case Kind::kList:
                assert(typeNode.listItemType != nullptr);
                walkType(state.next(typeNode.listItemType.get(), newPath));
                break;

            case Kind::kMap:
                assert(typeNode.mapKeyType != nullptr);
                assert(typeNode.mapValueType != nullptr);
                walkType(state.next(typeNode.mapKeyType.get(), newPath));
                walkType(state.next(typeNode.mapValueType.get(), newPath));
                break;

            case Kind::kSet:
                assert(typeNode.setKeyType != nullptr);
                walkType(state.next(typeNode.setKeyType.get(), newPath));
                break;

            case Kind::kOptional:
                assert(typeNode.optionalValueType != nullptr);
                walkType(state.next(typeNode.optionalValueType.get(), newPath));
                break;

            default:
                // Nothing to do for other types.
                break;
        }
    } else {
        // This is a named record type. Could be either a built-in type or a user-defined type.
        const auto& typeName = *state.name;
        const auto typeNameLowercase = shared::to_lower_copy(*state.name);
        if (findBuiltInRecordType(typeNameLowercase, nullptr)) {
            // This is a built-in type. We know there are no cycles in the built-in types so we don't need to check.
            return;
        }

        const auto newPath = state.path.push_back(typeNameLowercase);

        // Check if the type is already in the path. If it is, this is a cycle.
        if (std::find(state.path.begin(), state.path.end(), typeNameLowercase) != state.path.end()) {
            reportRecursiveTypeError(typeName, newPath, Token{});  // TODO: Need a token here
        }

        // It must be a user-defined type which we need to check. We've already checked that it exists.
        const auto& compiledUserType = *state.compiledProgram->userTypesByNameLowercase.at(typeNameLowercase);
        for (const auto& field : compiledUserType.fields) {
            walkType(state.next(field->parameterNode->type.get(), newPath));
        }
    }
}

void checkType(size_t sourceMemberIndex, CompiledProgram* compiledProgram) {
    const auto& compiledUserType = *compiledProgram->userTypesBySourceMemberIndex.find(sourceMemberIndex)->second;
    immer::flex_vector<std::string> const path{};
    State const state{ compiledUserType.name, path, compiledProgram };
    walkType(state);
}

}  // namespace

void checkRecursiveTypes(const SourceProgram& sourceProgram, CompiledProgram* compiledProgram) {
    // Check each named record type.
    sourceProgram.forEachMemberIndex(
        SourceMemberType::kType,
        [compiledProgram](const SourceMember& /*sourceMember*/, auto index) { checkType(index, compiledProgram); });
}

}  // namespace compiler
