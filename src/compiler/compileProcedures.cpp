#include "compileProcedures.h"
#include "CompilerException.h"
#include "bindProcedureSymbols.h"
#include "emit.h"
#include "parse.h"
#include "tokenize.h"
#include "typeCheck.h"
#include "util/cast.h"
#include "vm/Opcode.h"

using util::dynamic_cast_move;

namespace compiler {

class VariableIndexingState {
   public:
    int nextLocalValueIndex = 0;
    int nextLocalObjectIndex = 0;
};

static void assignLocalVariableIndexToSymbolDeclaration(Node* symbolDeclarationNode, VariableIndexingState* state) {
    auto typeNode = symbolDeclarationNode->getSymbolDeclarationType();
    if (typeNode == nullptr) {
        throw CompilerException(
            fmt::format(
                "Internal error. getSymbolDeclarationType() returned null. Symbol name is \"{}\".",
                *symbolDeclarationNode->getSymbolDeclaration()),
            symbolDeclarationNode->token);
    }
    if (typeNode->isValueType()) {
        symbolDeclarationNode->localValueIndex = state->nextLocalValueIndex++;
    } else {
        symbolDeclarationNode->localObjectIndex = state->nextLocalObjectIndex++;
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

static void compileProcedure(const SourceMember& sourceMember, CompiledProgram* compiledProgram) {
    auto tokens = tokenize(sourceMember.source, TokenizeType::kCompile, &sourceMember);
    auto parserResult = parse(&sourceMember, ParserRootProduction::kMember, tokens);
    if (!parserResult.isSuccess) {
        throw CompilerException(parserResult.message, *parserResult.token);
    }
    if (parserResult.node->getMemberType() != MemberType::kProcedure) {
        throw CompilerException("This member must be a subroutine or function.", tokens[0]);
    }
    auto procedureNode = dynamic_cast_move<ProcedureNode>(std::move(parserResult.node));
    bindProcedureSymbols(procedureNode.get(), *compiledProgram);
    typeCheck(procedureNode.get());
    int numLocalValues = 0;
    int numLocalObjects = 0;
    assignLocalVariableIndices(procedureNode.get(), &numLocalValues, &numLocalObjects);
    auto pcode = emit(*procedureNode, numLocalValues, numLocalObjects);
    auto vmProcedure = std::make_unique<vm::Procedure>();
    vmProcedure->instructions = std::move(pcode);
    compiledProgram->vmProgram.procedures.push_back(std::move(vmProcedure));
}

void compileProcedures(const SourceProgram& sourceProgram, CompiledProgram* compiledProgram) {
    sourceProgram.forEachMember(SourceMemberType::kProcedure, [compiledProgram](const SourceMember& sourceMember) {
        compileProcedure(sourceMember, compiledProgram);
    });
}

}  // namespace compiler
