#include "compiler/compileProcedures.h"
#include "compiler/bindNamedRecordTypes.h"
#include "compiler/bindProcedureSymbols.h"
#include "compiler/bindYieldStatements.h"
#include "compiler/BuiltInConstantList.h"
#include "compiler/BuiltInProcedureList.h"
#include "compiler/checkMissingReturns.h"
#include "compiler/CompilerException.h"
#include "compiler/emit.h"
#include "compiler/fixDottedExpressionFunctionCalls.h"
#include "compiler/parse.h"
#include "compiler/tokenize.h"
#include "compiler/typeCheck.h"
#include "shared/cast.h"
#include "shared/Opcode.h"
#include "shared/strings.h"

using shared::dynamic_cast_move;

namespace compiler {

class VariableIndexingState {
   public:
    int nextLocalValueIndex = 0;
    int nextLocalObjectIndex = 0;
};

static void assignLocalVariableIndexToSymbolDeclaration(Node* symbolDeclarationNode, VariableIndexingState* state) {
    auto typeNode = symbolDeclarationNode->getSymbolDeclarationType();
    assert(typeNode != nullptr);
    if (typeNode->isValueType()) {
        symbolDeclarationNode->localValueIndex = state->nextLocalValueIndex++;
    } else {
        symbolDeclarationNode->localObjectIndex = state->nextLocalObjectIndex++;
    }
}

static void assignLocalVariableIndicesForStatementTemporaries(
    StatementNode* statementNode,
    int numValues,
    int numObjects,
    VariableIndexingState* state) {
    if (numValues > 0) {
        statementNode->tempLocalValueIndex = state->nextLocalValueIndex;
        state->nextLocalValueIndex += numValues;
    }
    if (numObjects > 0) {
        statementNode->tempLocalObjectIndex = state->nextLocalObjectIndex;
        state->nextLocalObjectIndex += numObjects;
    }
}

static void assignLocalVariableIndicesInBody(BodyNode* body, VariableIndexingState* state) {
    for (auto& statement : body->statements) {
        // does this statement declare a symbol?
        auto symbolDeclaration = statement->getSymbolDeclaration();
        if (symbolDeclaration.has_value()) {
            assignLocalVariableIndexToSymbolDeclaration(statement.get(), state);
        }

        // does it a declare a second symbol in a sub-node?
        auto* childSymbolDeclarationNode = statement->getChildSymbolDeclaration();
        if (childSymbolDeclarationNode != nullptr) {
            assignLocalVariableIndexToSymbolDeclaration(childSymbolDeclarationNode, state);
        }

        // does it need temporary variables?
        auto numTempValues = statement->getTempLocalValueCount();
        auto numTempObjects = statement->getTempLocalObjectCount();
        assignLocalVariableIndicesForStatementTemporaries(statement.get(), numTempValues, numTempObjects, state);

        // does it have sub-statements?
        statement->visitBodies([state](BodyNode* body) -> bool {
            assignLocalVariableIndicesInBody(body, state);
            return true;
        });
    }
}

static void assignLocalVariableIndices(ProcedureNode* procedure, int* numLocalValues, int* numLocalObjects) {
    VariableIndexingState state{};
    assignLocalVariableIndicesInBody(procedure->body.get(), &state);
    *numLocalValues = state.nextLocalValueIndex;
    *numLocalObjects = state.nextLocalObjectIndex;
}

static void assignArgumentIndices(ProcedureNode* procedure) {
    auto nextValueIndex = 0;
    auto nextObjectIndex = 0;
    for (auto& parameter : procedure->parameters) {
        assert(parameter->type != nullptr);
        if (parameter->type->isValueType()) {
            parameter->argumentValueIndex = nextValueIndex++;
        } else {
            parameter->argumentObjectIndex = nextObjectIndex++;
        }
    }
}

// We do pass 1 on all procedures before proceeding to pass 2.
static void compileProcedurePass1(
    CompiledProgram* compiledProgram,
    CompiledProcedure* compiledProcedure,
    SymbolScope* globalSymbolScope,
    const BuiltInProcedureList& builtInProcedures) {
    auto* procedureNode = compiledProcedure->procedureNode.get();
    checkMissingReturns(procedureNode);
    fixDottedExpressionFunctionCalls(procedureNode, builtInProcedures, *compiledProgram);
    bindProcedureSymbols(globalSymbolScope, procedureNode);
    bindYieldStatements(procedureNode);
    bindNamedRecordTypes(procedureNode, *compiledProgram);
}

// Pass 2 happens after pass 1 is complete for all procedures.
static void compileProcedurePass2(
    const SourceProgram& sourceProgram,
    CompiledProgram* compiledProgram,
    CompiledProcedure* compiledProcedure,
    const BuiltInProcedureList& builtInProcedures) {
    auto* procedureNode = compiledProcedure->procedureNode.get();
    typeCheck(procedureNode, sourceProgram, compiledProgram, builtInProcedures);
    int numLocalValues = 0;
    int numLocalObjects = 0;
    assignLocalVariableIndices(procedureNode, &numLocalValues, &numLocalObjects);
    assignArgumentIndices(procedureNode);

    auto pcode = emit(*procedureNode, numLocalValues, numLocalObjects, compiledProgram);
    compiledProgram->vmProcedures.push_back(std::move(pcode));
}

void assignProcedureIndices(const SourceProgram& sourceProgram, CompiledProgram* compiledProgram) {
    size_t nextProcedureIndex = 0;

    sourceProgram.forEachMemberIndex(
        SourceMemberType::kProcedure,
        [compiledProgram, &nextProcedureIndex](const SourceMember& /*sourceMember*/, size_t index) {
            auto procedureIndex = nextProcedureIndex++;
            auto compiledProcedure = std::make_unique<CompiledProcedure>();
            compiledProcedure->sourceMemberIndex = index;
            compiledProcedure->procedureIndex = procedureIndex;
            compiledProgram->procedures.push_back(std::move(compiledProcedure));
        });
}

void compileProcedures(const SourceProgram& sourceProgram, CompiledProgram* compiledProgram) {
    BuiltInConstantList const builtInConstants{};
    BuiltInProcedureList const builtInProcedures{};
    SymbolScope globalSymbolScope{ *compiledProgram };

    // add symbols to the global scope for built-in constants
    for (const auto& builtInConstant : builtInConstants.constants) {
        globalSymbolScope.addSymbol(builtInConstant.second.get(), SymbolType::kVariable);
    }

    // add symbols to the global scope for built-in procedures
    for (const auto& builtInProcedureGroup : builtInProcedures.map) {
        for (auto& builtInProcedure : *builtInProcedureGroup.second) {
            globalSymbolScope.addSymbol(builtInProcedure.get(), SymbolType::kProcedure);
        }
    }

    // add symbols to the global scope for user-defined constants and global variables
    for (auto& compiledGlobalVariable : compiledProgram->globalVariables) {
        globalSymbolScope.addSymbol(compiledGlobalVariable->dimOrConstStatementNode.get(), SymbolType::kVariable);
    }

    assignProcedureIndices(sourceProgram, compiledProgram);

    // tokenize and parse each procedure so we have the names
    std::optional<size_t> mainProcedureIndex{};
    for (auto& compiledProcedure : compiledProgram->procedures) {
        auto& sourceMember = *sourceProgram.members.at(compiledProcedure->sourceMemberIndex);
        auto tokens = tokenize(sourceMember.source, TokenizeType::kCompile, &sourceMember);
        auto parserResult = parse(&sourceMember, ParserRootProduction::kMember, tokens);
        if (!parserResult.isSuccess) {
            throw CompilerException(CompilerErrorCode::kSyntax, parserResult.message, *parserResult.token);
        }
        if (parserResult.node->getMemberType() != MemberType::kProcedure) {
            throw CompilerException(
                CompilerErrorCode::kWrongMemberType, "This member must be a subroutine or function.", tokens[0]);
        }
        auto procedureNode = dynamic_cast_move<ProcedureNode>(std::move(parserResult.node));
        if (shared::to_lower_copy(procedureNode->name) == "main") {
            mainProcedureIndex = compiledProcedure->procedureIndex;
        }
        procedureNode->procedureIndex = compiledProcedure->procedureIndex;
        compiledProcedure->name = procedureNode->name;
        compiledProcedure->nameLowercase = shared::to_lower_copy(procedureNode->name);
        compiledProcedure->procedureNode = std::move(procedureNode);
    }

    // we need a Main sub
    if (!mainProcedureIndex.has_value()) {
        throw CompilerException(
            CompilerErrorCode::kMissingMainSub, "There is no \"Main\" subroutine in this program.", {});
    }

    // add symbols to the global scope for user procedures
    for (auto& compiledProcedure : compiledProgram->procedures) {
        globalSymbolScope.addSymbol(compiledProcedure->procedureNode.get(), SymbolType::kProcedure);
    }

    // compile each procedure - two passes
    for (auto& compiledProcedure : compiledProgram->procedures) {
        compileProcedurePass1(compiledProgram, compiledProcedure.get(), &globalSymbolScope, builtInProcedures);
    }

    for (auto& compiledProcedure : compiledProgram->procedures) {
        compileProcedurePass2(sourceProgram, compiledProgram, compiledProcedure.get(), builtInProcedures);
    }

    compiledProgram->vmStartupProcedureIndex = *mainProcedureIndex;
}

}  // namespace compiler
