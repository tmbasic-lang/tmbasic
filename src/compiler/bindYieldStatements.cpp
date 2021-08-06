#include "bindYieldStatements.h"
#include "ast.h"

namespace compiler {

static void bindYieldStatementsInBody(BodyNode* body, Node* target) {
    auto* vec = target->getYieldStatementNodesList();
    for (auto& statement : body->statements) {
        auto statementType = statement->getStatementType();
        if (statementType == StatementType::kYield) {
            auto* selectNode = &static_cast<YieldStatementNode&>(*statement);
            selectNode->boundCollectionDeclaration = target;
            vec->push_back(selectNode);
        } else if (statementType == StatementType::kDimList) {
            auto* dimListNode = &static_cast<DimListStatementNode&>(*statement);
            bindYieldStatementsInBody(dimListNode->body.get(), dimListNode);
        } else if (statementType == StatementType::kDimMap) {
            auto* dimMapNode = &static_cast<DimMapStatementNode&>(*statement);
            bindYieldStatementsInBody(dimMapNode->body.get(), dimMapNode);
        } else {
            statement->visitBodies([target](auto* body) -> bool {
                bindYieldStatementsInBody(body, target);
                return true;
            });
        }
    }
}

void bindYieldStatements(ProcedureNode* procedure) {
    bindYieldStatementsInBody(procedure->body.get(), procedure);
}

}  // namespace compiler
