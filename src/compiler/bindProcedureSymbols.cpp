#include "bindProcedureSymbols.h"

using basic::BodyNode;
using basic::ExpressionNode;
using basic::GlobalVariableNode;
using basic::Node;
using basic::ProcedureNode;
using basic::StatementNode;
using basic::SymbolReferenceExpressionNode;

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
                return &found->second;
            }
        }
        return nullptr;
    }

    AddSymbolResult addSymbol(const Node& symbolDeclaration) {
        auto optionalName = symbolDeclaration.getSymbolDeclaration();
        if (!optionalName.has_value()) {
            return AddSymbolResult::kNoSymbolDeclaration;
        }

        auto lowercaseName = boost::algorithm::to_lower_copy(*optionalName);
        const auto* existingSymbolDeclaration = lookup(lowercaseName);
        if (existingSymbolDeclaration != nullptr) {
            return AddSymbolResult::kDuplicateName;
        }
        _symbolDeclarations.insert({ { lowercaseName, symbolDeclaration } });
        return AddSymbolResult::kSuccess;
    }

   private:
    const Scope* _parentScope = nullptr;
    std::unordered_map<std::string, const Node&> _symbolDeclarations;
};

static Scope makeProcedureGlobalScope(ProcedureNode* procedure, const vm::Program& program) {
    auto scope = Scope();
    for (size_t i = 0; i < program.globalVariables.size(); i++) {
        const auto& globalVariable = program.globalVariables[i];
        auto node = std::make_unique<GlobalVariableNode>(globalVariable->name, i);
        auto result = scope.addSymbol(*node);
        assert(result == AddSymbolResult::kSuccess);
        procedure->globalVariables.push_back(std::move(node));
    }
    return scope;
}

static CompilerResult bindSymbol(Node* node, Scope* parentScope, Scope* childScope) {
    auto* symbolScope = node->isSymbolVisibleToSiblingStatements() ? parentScope : childScope;
    auto result = symbolScope->addSymbol(*node);
    assert(result != AddSymbolResult::kNoSymbolDeclaration);
    if (result == AddSymbolResult::kDuplicateName) {
        std::ostringstream s;
        s << "There is already a variable named \"" << *node->getSymbolDeclaration()
          << "\". Try another name for this variable.";
        return CompilerResult::error(s.str(), node->token);
    }
    return CompilerResult::success();
}

static CompilerResult bindExpressionSymbols(ExpressionNode* node, Scope* scope) {
    if (node->isSymbolReference()) {
        auto* symbolRef = dynamic_cast<SymbolReferenceExpressionNode*>(node);
        auto lowercaseName = boost::algorithm::to_lower_copy(symbolRef->name);
        const auto* symbolDeclaration = scope->lookup(lowercaseName);
        if (symbolDeclaration != nullptr) {
            symbolRef->boundSymbolDeclaration = symbolDeclaration;
        } else {
            std::ostringstream s;
            s << "There is no variable named \"" << symbolRef->name << "\" accessible from here.";
            return CompilerResult::error(s.str(), node->token);
        }
    }
    return CompilerResult::success();
}

static CompilerResult bindBodySymbols(BodyNode* node, Scope* scope);

static CompilerResult bindStatementSymbols(StatementNode* node, Scope* scope) {
    auto result = CompilerResult::success();
    auto subScope = Scope(scope);

    // does this statement declare a symbol?
    auto symbolDeclaration = node->getSymbolDeclaration();
    if (symbolDeclaration.has_value()) {
        result = bindSymbol(node, scope, &subScope);
        if (!result.isSuccess) {
            return result;
        }
    }

    // does it a declare a second symbol in a sub-node?
    auto* childSymbolDeclarationNode = node->getChildSymbolDeclaration();
    if (childSymbolDeclarationNode != nullptr) {
        auto childSymbolDeclaration = childSymbolDeclarationNode->getSymbolDeclaration();
        if (childSymbolDeclaration.has_value()) {
            result = bindSymbol(childSymbolDeclarationNode, scope, &subScope);
            if (!result.isSuccess) {
                return result;
            }
        }
    }

    // does it have sub-expressions?
    node->visitExpressions([&subScope, &result](ExpressionNode& expr) -> bool {
        result = bindExpressionSymbols(&expr, &subScope);
        return result.isSuccess;
    });
    if (!result.isSuccess) {
        return result;
    }

    // does it have sub-statements?
    node->visitBodies([&subScope, &result](BodyNode& body) -> bool {
        result = bindBodySymbols(&body, &subScope);
        return result.isSuccess;
    });
    if (!result.isSuccess) {
        return result;
    }

    return CompilerResult::success();
}

CompilerResult bindBodySymbols(BodyNode* node, Scope* scope) {
    for (auto& statement : node->statements) {
        auto result = bindStatementSymbols(statement.get(), scope);
        if (!result.isSuccess) {
            return result;
        }
    }
    return CompilerResult::success();
}

CompilerResult bindProcedureSymbols(ProcedureNode* procedure, const vm::Program& program) {
    auto globalScope = makeProcedureGlobalScope(procedure, program);
    auto procedureScope = Scope(&globalScope);

    for (const auto& parameter : procedure->parameters) {
        auto result = procedureScope.addSymbol(*parameter);
        assert(result != AddSymbolResult::kNoSymbolDeclaration);
        if (result == AddSymbolResult::kDuplicateName) {
            std::ostringstream s;
            s << "This parameter \"" << parameter->name << "\" conflicts with a global variable of the same name. "
              << "Try using a different name for this parameter.";
            return CompilerResult::error(s.str(), parameter->token);
        }
    }

    auto bodyResult = bindBodySymbols(procedure->body.get(), &procedureScope);
    if (!bodyResult.isSuccess) {
        return bodyResult;
    }

    return CompilerResult::success();
}

}  // namespace compiler
