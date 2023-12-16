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

enum class SymbolType {
    kVariable,
    kProcedure,
};

class SymbolScope {
   public:
    explicit SymbolScope(const CompiledProgram& program);
    explicit SymbolScope(const SymbolScope* parentScope);

    const Node* lookup(const std::string& lowercaseName, SymbolType symbolType);
    AddSymbolResult addSymbol(Node* symbolDeclaration, SymbolType symbolType);

   private:
    SymbolScope();
    const SymbolScope* _parentScope = nullptr;
    std::unordered_map<std::string, Node*> _variableDeclarations;
    std::unordered_map<std::string, Node*> _procedureDeclarations;
};

void bindProcedureSymbols(SymbolScope* scope, ProcedureNode* procedure);

}  // namespace compiler
