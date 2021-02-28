#include "typeCheck.h"

namespace compiler {

CompilerResult typeCheckBody(BodyNode* bodyNode) {
    auto result = CompilerResult::success();

    for (auto& statementNode : bodyNode->statements) {
        statementNode->visitExpressions([](ExpressionNode& /*unused*/) -> bool { return true; });
    }

    return result;
}

CompilerResult typeCheck(ProcedureNode* procedureNode) {
    return typeCheckBody(procedureNode->body.get());
}

};  // namespace compiler
