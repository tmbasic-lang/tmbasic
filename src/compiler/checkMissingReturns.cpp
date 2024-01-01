#include "compiler/checkMissingReturns.h"
#include "compiler/CompilerException.h"

namespace compiler {

static bool findExitDoInBody(BodyNode* body) {
    for (auto& statement : body->statements) {
        switch (statement->getStatementType()) {
            case StatementType::kDo:
                // Don't recurse into a nested Do loop. Any Exit Do inside there will exit from that nested loop.
                break;

            case StatementType::kExit: {
                const auto& exitStatement = dynamic_cast<const ExitStatementNode&>(*statement);
                if (exitStatement.scope == LoopType::kDo) {
                    // Found an Exit Do in this body.
                    return true;
                }
                break;
            }

            default: {
                auto found = false;
                statement->visitBodies([&](BodyNode* body) -> bool {
                    if (findExitDoInBody(body)) {
                        found = true;
                        return false;  // Stop visiting bodies.
                    }
                    return true;  // Continue to the next body.
                });
                if (found) {
                    //  Found an Exit Do in this body.
                    return true;
                }
                break;
            }
        }
    }

    return false;
}

static bool checkBody(BodyNode* body);

static bool checkIfStatement(IfStatementNode* ifStatement) {
    // The only way an if statement can always return is if there's an else body and all bodies return.
    if (ifStatement->elseBody == nullptr) {
        return false;
    }

    if (!checkBody(ifStatement->body.get())) {
        return false;
    }

    for (const auto& elseIf : ifStatement->elseIfs) {
        if (!checkBody(elseIf->body.get())) {
            return false;
        }
    }

    return checkBody(ifStatement->elseBody.get());
}

static bool checkDoStatement(DoStatementNode* doStatement) {
    if (findExitDoInBody(doStatement->body.get())) {
        // There's an Exit Do inside this body, so all bets are off.
        return false;
    }

    return checkBody(doStatement->body.get());
}

static bool checkDimListStatement(DimListStatementNode* dimListStatement) {
    return checkBody(dimListStatement->body.get());
}

static bool checkDimMapStatement(DimMapStatementNode* dimMapStatement) {
    return checkBody(dimMapStatement->body.get());
}

static bool checkDimSetStatement(DimSetStatementNode* dimSetStatement) {
    return checkBody(dimSetStatement->body.get());
}

static bool checkSelectCaseStatement(SelectCaseStatementNode* selectCaseStatement) {
    // The Select Case must have a Case Else block, and every block must return.
    auto foundCaseElseBlock = false;

    for (const auto& c : selectCaseStatement->cases) {
        if (c->values.empty()) {
            foundCaseElseBlock = true;
            break;
        }
    }

    if (!foundCaseElseBlock) {
        return false;
    }

    for (const auto& c : selectCaseStatement->cases) {
        if (!checkBody(c->body.get())) {
            return false;
        }
    }

    return true;
}

static bool checkTryStatement(TryStatementNode* tryStatement) {
    // Both Try and Catch blocks must return.
    return checkBody(tryStatement->tryBody.get()) && checkBody(tryStatement->catchBody.get());
}

/*static*/ bool checkBody(BodyNode* body) {
    for (const auto& statement : body->statements) {
        switch (statement->getStatementType()) {
            case StatementType::kReturn:
            case StatementType::kThrow:
            case StatementType::kRethrow:
                // Anything after this point is dead code. This is effectively the end of the body.
                return true;

            case StatementType::kExit:
                // Control can leave this body via this Exit statement so no need to look further.
                // Note: this isn't enough by itself; when checking the Do loop itself, we scan for any Exit Do.
                // Consider a For containing an If containing an Exit For, and then a seemingly unconditional return
                // at the end. The Exit must taint the loop.
                return false;

            case StatementType::kIf:
                if (checkIfStatement(dynamic_cast<IfStatementNode*>(statement.get()))) {
                    return true;
                }
                break;

            case StatementType::kDo: {
                if (checkDoStatement(dynamic_cast<DoStatementNode*>(statement.get()))) {
                    return true;
                }
                break;
            }

            case StatementType::kWhile:
            case StatementType::kFor:
            case StatementType::kForEach:
                // Careful! It doesn't matter what's in the body of these statements, because there's a possibility
                // that the body will not execute at all. Thus these loops can never be guaranteed to return.
                break;

            case StatementType::kDimList:
                if (checkDimListStatement(dynamic_cast<DimListStatementNode*>(statement.get()))) {
                    return true;
                }
                break;

            case StatementType::kDimMap:
                if (checkDimMapStatement(dynamic_cast<DimMapStatementNode*>(statement.get()))) {
                    return true;
                }
                break;

            case StatementType::kDimSet:
                if (checkDimSetStatement(dynamic_cast<DimSetStatementNode*>(statement.get()))) {
                    return true;
                }
                break;

            case StatementType::kSelectCase:
                if (checkSelectCaseStatement(dynamic_cast<SelectCaseStatementNode*>(statement.get()))) {
                    return true;
                }
                break;

            case StatementType::kTry:
                if (checkTryStatement(dynamic_cast<TryStatementNode*>(statement.get()))) {
                    return true;
                }
                break;

            default:
                break;
        }
    }

    return false;
}

void checkMissingReturns(ProcedureNode* procedure) {
    if (procedure->returnType == nullptr) {
        // Subroutines don't need return statements.
        return;
    }

    if (!checkBody(procedure->body.get())) {
        throw CompilerException(
            CompilerErrorCode::kControlReachesEndOfFunction,
            "Control may reach the end of this function without returning or throwing.", procedure->token);
    }
}

}  // namespace compiler
