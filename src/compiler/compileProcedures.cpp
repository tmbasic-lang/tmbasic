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

static void compileProcedure(
    size_t sourceMemberIndex,
    const SourceMember& sourceMember,
    CompiledProgram* compiledProgram,
    bool* isMain) {
    auto tokens = tokenize(sourceMember.source, TokenizeType::kCompile, &sourceMember);
    auto parserResult = parse(&sourceMember, ParserRootProduction::kMember, tokens);
    if (!parserResult.isSuccess) {
        throw CompilerException(parserResult.message, *parserResult.token);
    }
    if (parserResult.node->getMemberType() != MemberType::kProcedure) {
        throw CompilerException("This member must be a subroutine or function.", tokens[0]);
    }
    auto procedureNode = dynamic_cast_move<ProcedureNode>(std::move(parserResult.node));
    *isMain = boost::algorithm::to_lower_copy(procedureNode->name) == "main";
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

void assignProcedureIndices(const SourceProgram& sourceProgram, CompiledProgram* compiledProgram) {
    size_t nextProcedureIndex = 0;

    sourceProgram.forEachMemberIndex(
        SourceMemberType::kProcedure,
        [compiledProgram, &nextProcedureIndex](const SourceMember& sourceMember, size_t index) {
            auto procedureIndex = nextProcedureIndex++;
            compiledProgram->sourceMemberIndexToProcedureIndex.emplace(index, procedureIndex);
            compiledProgram->procedureIndexToSourceMemberIndex.emplace(procedureIndex, index);
        });
}

void compileProcedures(const SourceProgram& sourceProgram, CompiledProgram* compiledProgram) {
    assignProcedureIndices(sourceProgram, compiledProgram);

    std::optional<size_t> mainSourceMemberIndex;

    sourceProgram.forEachMemberIndex(
        SourceMemberType::kProcedure,
        [compiledProgram, &mainSourceMemberIndex](const SourceMember& sourceMember, size_t index) {
            bool isMain = false;
            compileProcedure(index, sourceMember, compiledProgram, &isMain);
            if (isMain) {
                mainSourceMemberIndex = index;
            }
        });

    if (!mainSourceMemberIndex.has_value()) {
        throw CompilerException("There is no \"Main\" subroutine in this program.", {});
    }

    compiledProgram->vmProgram.startupProcedureIndex =
        compiledProgram->sourceMemberIndexToProcedureIndex.find(*mainSourceMemberIndex)->second;
}

}  // namespace compiler
