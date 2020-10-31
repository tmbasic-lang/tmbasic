#include "bindProcedureSymbols.h"

using namespace basic;

namespace compiler {

enum class AddSymbolResult { kSuccess, kNoSymbolDeclaration, kDuplicateName };

class Scope {
   public:
    const Scope* parentScope = nullptr;
    std::unordered_map<std::string, Node*> symbolDeclarations;

    Scope() {}
    Scope(const Scope& parentScope) : parentScope(&parentScope) {}

    Node* lookup(std::string lowercaseName) {
        const auto* scope = this;
        while (scope) {
            auto found = scope->symbolDeclarations.find(lowercaseName);
            if (found == scope->symbolDeclarations.end()) {
                scope = scope->parentScope;
            } else {
                return found->second;
            }
        }
        return nullptr;
    }

    AddSymbolResult addSymbol(Node* symbolDeclaration) {
        auto optionalName = symbolDeclaration->getSymbolDeclaration();
        if (!optionalName.has_value()) {
            return AddSymbolResult::kNoSymbolDeclaration;
        }

        auto lowercaseName = boost::algorithm::to_lower_copy(*optionalName);
        auto* existingSymbolDeclaration = lookup(lowercaseName);
        if (existingSymbolDeclaration) {
            return AddSymbolResult::kDuplicateName;
        } else {
            symbolDeclarations.insert({ { lowercaseName, symbolDeclaration } });
            return AddSymbolResult::kSuccess;
        }
    }
};

/*static std::unique_ptr<Scope> makeRootScope(const vm::Program& program) {
    auto scope = std::make_unique<Scope>();

    for (const auto& procedure : program.procedures) {
//TODO
    }

    for (const auto& globalVariable : program.globalVariables) {
//TODO
    }

    return scope;
}*/

CompilerResult bindProcedureSymbols(basic::ProcedureNode& procedure, const vm::Program& program) {
    assert(false);  // TODO
    return CompilerResult::success();
}

}  // namespace compiler
