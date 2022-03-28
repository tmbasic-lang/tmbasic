#include "fixDottedExpressionFunctionCalls.h"
#include "CompilerException.h"

namespace compiler {

class DottedExpressionFunctionCallFixer {
   private:
    const CompiledProgram& _compiledProgram;
    const BuiltInProcedureList& _builtInProcedures;

   public:
    DottedExpressionFunctionCallFixer(
        const CompiledProgram& compiledProgram,
        const BuiltInProcedureList& builtInProcedures)
        : _compiledProgram(compiledProgram), _builtInProcedures(builtInProcedures) {}

    void fixProcedure(ProcedureNode* node) { fixBody(node->body.get()); }

    void fixBody(BodyNode* body) {
        for (auto& statement : body->statements) {
            fixStatement(statement.get());
        }
    }

    void fixStatement(StatementNode* statement) {
        statement->visitExpressions(/* rootsOnly: */ false, [this](ExpressionNode* expressionNode) {
            fixExpression(expressionNode);
            return true;
        });

        statement->visitBodies([this](BodyNode* bodyNode) {
            fixBody(bodyNode);
            return true;
        });
    }

    void fixExpression(ExpressionNode* expression) {
        if (expression->getExpressionType() == ExpressionType::kDotted) {
            auto* dotted = dynamic_cast<DottedExpressionNode*>(expression);
            assert(dotted != nullptr);
            fixDottedExpression(dotted);
        }
    }

    void fixDottedExpression(DottedExpressionNode* dotted) {
        // does this look like SymbolRef(...)?
        auto hasCallShape = dotted->base->getExpressionType() == ExpressionType::kSymbolReference &&
            !dotted->dottedSuffixes.empty() && !dotted->dottedSuffixes.at(0)->name.has_value();
        if (!hasCallShape) {
            return;
        }

        // is there a function with this name?
        // Note: fixDottedExpressionFunctionCalls() and typeCheckCall() duplicate the procedure lookup logic
        auto& symbolReference = dynamic_cast<SymbolReferenceExpressionNode&>(*dotted->base);
        auto lowercaseProcedureName = boost::to_lower_copy(symbolReference.name);
        auto functionExists = false;
        for (const auto& compiledProcedure : _compiledProgram.procedures) {
            if (compiledProcedure->nameLowercase == lowercaseProcedureName) {
                functionExists = true;
                break;
            }
        }
        for (const auto& builtInProcedure : _builtInProcedures.get(symbolReference.name)) {
            (void)builtInProcedure;
            functionExists = true;
            break;
        }

        // if so, convert the base and the first dotted suffix into a function call expression
        if (functionExists) {
            fixDottedExpressionWithFunctionCallBase(dotted, &symbolReference);
        }
    }

    static void fixDottedExpressionWithFunctionCallBase(
        DottedExpressionNode* dotted,
        SymbolReferenceExpressionNode* symbolReference) {
        auto suffix = std::move(dotted->dottedSuffixes.at(0));
        dotted->dottedSuffixes.erase(dotted->dottedSuffixes.begin());
        dotted->base = std::make_unique<FunctionCallExpressionNode>(
            std::move(symbolReference->name), std::move(suffix->collectionIndexOrCallArgs),
            std::move(symbolReference->token));
    }
};

void fixDottedExpressionFunctionCalls(
    ProcedureNode* procedureNode,
    const BuiltInProcedureList& builtInProcedures,
    const CompiledProgram& compiledProgram) {
    DottedExpressionFunctionCallFixer fixer{ compiledProgram, builtInProcedures };
    fixer.fixProcedure(procedureNode);
}

}  // namespace compiler
