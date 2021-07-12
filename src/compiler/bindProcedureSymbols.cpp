#include "bindProcedureSymbols.h"
#include "CompilerException.h"

namespace compiler {

enum class AddSymbolResult { kSuccess, kNoSymbolDeclaration, kDuplicateName };

class Scope {
   public:
    Scope() = default;
    explicit Scope(const Scope* parentScope) : _parentScope(parentScope) {}

    const Node* lookup(const std::string& lowercaseName) {
        const auto* scope = this;
        while (scope != nullptr) {
            auto found = scope->_symbolDeclarations.find(lowercaseName);
            if (found == scope->_symbolDeclarations.end()) {
                scope = scope->_parentScope;
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
        const auto* existingSymbolDeclaration = lookup(lowercaseName);
        if (existingSymbolDeclaration != nullptr) {
            return AddSymbolResult::kDuplicateName;
        }
        _symbolDeclarations.insert(std::make_pair(lowercaseName, symbolDeclaration));
        return AddSymbolResult::kSuccess;
    }

   private:
    const Scope* _parentScope = nullptr;
    std::unordered_map<std::string, Node*> _symbolDeclarations;
};

static Scope makeProcedureGlobalScope(ProcedureNode* procedure, const CompiledProgram& program) {
    auto scope = Scope();
    for (const auto& globalVariable : program.globalVariables) {
        auto node = std::make_unique<GlobalVariableNode>(globalVariable->lowercaseName);
        auto result = scope.addSymbol(node.get());
        assert(result == AddSymbolResult::kSuccess);
        (void)result;  // avoid unused variable error in release builds
        procedure->globalVariables.push_back(std::move(node));
    }
    return scope;
}

static void bindSymbol(Node* node, Scope* parentScope, Scope* childScope) {
    auto* symbolScope = node->isSymbolVisibleToSiblingStatements() ? parentScope : childScope;
    auto result = symbolScope->addSymbol(node);
    assert(result != AddSymbolResult::kNoSymbolDeclaration);
    if (result == AddSymbolResult::kDuplicateName) {
        std::ostringstream s;
        s << "There is already a variable named \"" << *node->getSymbolDeclaration()
          << "\". Try another name for this variable.";
        throw CompilerException(s.str(), node->token);
    }
}

static void bindExpressionSymbols(ExpressionNode* node, Scope* scope) {
    if (node->isSymbolReference()) {
        auto* symbolRef = dynamic_cast<SymbolReferenceExpressionNode*>(node);
        auto lowercaseName = boost::algorithm::to_lower_copy(symbolRef->name);
        const auto* symbolDeclaration = scope->lookup(lowercaseName);
        if (symbolDeclaration != nullptr) {
            symbolRef->boundSymbolDeclaration = symbolDeclaration;
        } else {
            std::ostringstream s;
            s << "There is no variable named \"" << symbolRef->name << "\" accessible from here.";
            throw CompilerException(s.str(), node->token);
        }
    }
}

static void bindBodySymbols(BodyNode* node, Scope* scope);

static void bindStatementSymbols(StatementNode* node, Scope* scope) {
    auto subScope = Scope(scope);

    // does this statement declare a symbol?
    auto symbolDeclaration = node->getSymbolDeclaration();
    if (symbolDeclaration.has_value()) {
        bindSymbol(node, scope, &subScope);
    }

    // does it a declare a second symbol in a sub-node?
    auto* childSymbolDeclarationNode = node->getChildSymbolDeclaration();
    if (childSymbolDeclarationNode != nullptr) {
        auto childSymbolDeclaration = childSymbolDeclarationNode->getSymbolDeclaration();
        if (childSymbolDeclaration.has_value()) {
            bindSymbol(childSymbolDeclarationNode, scope, &subScope);
        }
    }

    // does it have sub-expressions?
    node->visitExpressions([&subScope](ExpressionNode* expr) -> bool {
        bindExpressionSymbols(expr, &subScope);
        return true;
    });

    // does it have sub-statements?
    node->visitBodies([&subScope](BodyNode* body) -> bool {
        bindBodySymbols(body, &subScope);
        return true;
    });
}

void bindBodySymbols(BodyNode* node, Scope* scope) {
    for (auto& statement : node->statements) {
        bindStatementSymbols(statement.get(), scope);
    }
}

void bindProcedureSymbols(ProcedureNode* procedure, const CompiledProgram& program) {
    auto globalScope = makeProcedureGlobalScope(procedure, program);
    auto procedureScope = Scope(&globalScope);

    for (const auto& parameter : procedure->parameters) {
        auto result = procedureScope.addSymbol(parameter.get());
        assert(result != AddSymbolResult::kNoSymbolDeclaration);
        if (result == AddSymbolResult::kDuplicateName) {
            std::ostringstream s;
            s << "This parameter \"" << parameter->name << "\" conflicts with a global variable of the same name. "
              << "Try using a different name for this parameter.";
            throw CompilerException(s.str(), parameter->token);
        }
    }

    bindBodySymbols(procedure->body.get(), &procedureScope);
}

}  // namespace compiler
