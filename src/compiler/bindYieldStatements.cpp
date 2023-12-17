#include "bindYieldStatements.h"
#include "CompilerException.h"
#include "ast.h"

namespace compiler {

static void bindYieldStatementsInBody(BodyNode* body, Node* target, bool insideDimCollection) {
    auto* vec = target->getYieldStatementNodesList();
    for (auto& statement : body->statements) {
        auto statementType = statement->getStatementType();
        if (statementType == StatementType::kYield) {
            if (!insideDimCollection) {
                throw CompilerException(
                    CompilerErrorCode::kYieldOutsideDimCollection,
                    "Yield must be inside a \"dim list\" or \"dim map\" block.", statement->token);
            }

            auto& yieldNode = dynamic_cast<YieldStatementNode&>(*statement);

            auto isYieldList = yieldNode.toExpression == nullptr;
            auto isYieldMap = !isYieldList;

            auto isDimList = dynamic_cast<DimListStatementNode*>(target) != nullptr;
            auto isDimMap = dynamic_cast<DimMapStatementNode*>(target) != nullptr;

            if (isDimList && !isYieldList) {
                throw CompilerException(
                    CompilerErrorCode::kInvalidYieldType,
                    "Inside \"dim list\" blocks, \"yield\" must take a single value.", statement->token);
            }

            if (isDimMap && !isYieldMap) {
                throw CompilerException(
                    CompilerErrorCode::kInvalidYieldType,
                    "Inside \"dim map\" blocks, \"yield\" must take a key and a value.", statement->token);
            }

            yieldNode.boundCollectionDeclaration = target;
            vec->push_back(&yieldNode);
        } else if (statementType == StatementType::kDimList) {
            auto* dimListNode = dynamic_cast<DimListStatementNode*>(statement.get());
            assert(dimListNode != nullptr);
            bindYieldStatementsInBody(dimListNode->body.get(), dimListNode, true);
        } else if (statementType == StatementType::kDimMap) {
            auto* dimMapNode = dynamic_cast<DimMapStatementNode*>(statement.get());
            assert(dimMapNode != nullptr);
            bindYieldStatementsInBody(dimMapNode->body.get(), dimMapNode, true);
        } else {
            statement->visitBodies([target, insideDimCollection](auto* body) -> bool {
                bindYieldStatementsInBody(body, target, insideDimCollection);
                return true;
            });
        }
    }
}

void bindYieldStatements(ProcedureNode* procedure) {
    bindYieldStatementsInBody(procedure->body.get(), procedure, false);
}

}  // namespace compiler
