#pragma once

#include "../common.h"
#include "compiler/CompiledProgram.h"
#include "compiler/ast.h"

namespace compiler {

enum class AddSymbolResult {
    kSuccess,
    kNoSymbolDeclaration,
    kDuplicateName,
};

class SymbolScope {
   public:
    explicit SymbolScope(const CompiledProgram& program);
    explicit SymbolScope(const SymbolScope* parentScope);

    const Node* lookup(const std::string& lowercaseName);
    AddSymbolResult addSymbol(Node* symbolDeclaration);

   private:
    SymbolScope();
    const SymbolScope* _parentScope = nullptr;
    std::unordered_map<std::string, Node*> _symbolDeclarations;
};

void bindProcedureSymbols(SymbolScope* scope, ProcedureNode* procedure);

}  // namespace compiler
