#include "bindProcedureSymbols.h"
#include "CompilerException.h"

namespace compiler {

SymbolScope::SymbolScope() = default;

SymbolScope::SymbolScope(const CompiledProgram& program) {
    for (const auto& globalVariable : program.globalVariables) {
        auto node = std::make_unique<GlobalVariableNode>(globalVariable->lowercaseName);
        auto result = addSymbol(node.get());
        assert(result == AddSymbolResult::kSuccess);
        (void)result;  // avoid unused variable error in release builds
    }
}

SymbolScope::SymbolScope(const SymbolScope* parentScope) : _parentScope(parentScope) {}

const Node* SymbolScope::lookup(const std::string& lowercaseName) {
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

AddSymbolResult SymbolScope::addSymbol(Node* symbolDeclaration) {
    auto optionalName = symbolDeclaration->getSymbolDeclaration();
    if (!optionalName.has_value()) {
        return AddSymbolResult::kNoSymbolDeclaration;
    }

    auto lowercaseName = boost::to_lower_copy(*optionalName);
    const auto* existingSymbolDeclaration = lookup(lowercaseName);
    if (existingSymbolDeclaration != nullptr) {
        return AddSymbolResult::kDuplicateName;
    }
    _symbolDeclarations.insert(std::make_pair(lowercaseName, symbolDeclaration));
    return AddSymbolResult::kSuccess;
}

static void bindSymbol(Node* node, SymbolScope* parentScope, SymbolScope* childScope) {
    auto* symbolScope = node->isSymbolVisibleToSiblingStatements() ? parentScope : childScope;
    auto result = symbolScope->addSymbol(node);
    assert(result != AddSymbolResult::kNoSymbolDeclaration);
    if (result == AddSymbolResult::kDuplicateName) {
        std::ostringstream s;
        s << "There is already a variable named \"" << *node->getSymbolDeclaration()
          << "\". Try another name for this variable.";
        throw CompilerException(CompilerErrorCode::kDuplicateSymbolName, s.str(), node->token);
    }
}

static void bindExpressionSymbols(ExpressionNode* node, SymbolScope* scope) {
    assert(node != nullptr);
    if (node->isSymbolReference()) {
        auto* symbolRef = dynamic_cast<SymbolReferenceExpressionNode*>(node);
        auto lowercaseName = boost::to_lower_copy(symbolRef->name);
        const auto* symbolDeclaration = scope->lookup(lowercaseName);
        if (symbolDeclaration != nullptr) {
            symbolRef->boundSymbolDeclaration = symbolDeclaration;
        } else {
            std::ostringstream s;
            s << "There is no variable named \"" << symbolRef->name << "\" accessible from here.";
            throw CompilerException(CompilerErrorCode::kSymbolNotFound, s.str(), node->token);
        }
    } else if (node->getExpressionType() == ExpressionType::kDotted) {
        auto& dotted = dynamic_cast<DottedExpressionNode&>(*node);
        bindExpressionSymbols(dotted.base.get(), scope);
        for (auto& suffix : dotted.dottedSuffixes) {
            for (auto& collectionIndexOrCallArg : suffix->collectionIndex) {
                bindExpressionSymbols(collectionIndexOrCallArg.get(), scope);
            }
        }
    }
}

static void bindBodySymbols(BodyNode* node, SymbolScope* scope);

static void bindStatementSymbols(StatementNode* node, SymbolScope* scope) {
    auto subScope = SymbolScope(scope);

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
    node->visitExpressions(false, [&subScope](ExpressionNode* expr) -> bool {
        bindExpressionSymbols(expr, &subScope);
        return true;
    });

    // does it have sub-statements?
    node->visitBodies([&subScope](BodyNode* body) -> bool {
        bindBodySymbols(body, &subScope);
        return true;
    });
}

static void bindBodySymbols(BodyNode* node, SymbolScope* scope) {
    for (auto& statement : node->statements) {
        bindStatementSymbols(statement.get(), scope);
    }
}

void bindProcedureSymbols(SymbolScope* scope, ProcedureNode* procedure) {
    SymbolScope procedureScope{ scope };

    for (const auto& parameter : procedure->parameters) {
        auto result = procedureScope.addSymbol(parameter.get());
        assert(result != AddSymbolResult::kNoSymbolDeclaration);
        if (result == AddSymbolResult::kDuplicateName) {
            std::ostringstream s;
            s << "This parameter \"" << parameter->name << "\" conflicts with a global variable of the same name. "
              << "Try using a different name for this parameter.";
            throw CompilerException(CompilerErrorCode::kDuplicateSymbolName, s.str(), parameter->token);
        }
    }

    bindBodySymbols(procedure->body.get(), &procedureScope);
}

}  // namespace compiler
