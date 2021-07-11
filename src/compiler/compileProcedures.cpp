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

static void assignLocalVariableIndicesInBody(BodyNode* body, VariableIndexingState* state) {
    for (auto& statement : body->statements) {
        // does this statement declare a symbol?
        auto symbolDeclaration = statement->getSymbolDeclaration();
        if (symbolDeclaration.has_value()) {
            throw std::runtime_error("not impl");
        }

        // does it a declare a second symbol in a sub-node?
        auto* childSymbolDeclarationNode = statement->getChildSymbolDeclaration();
        if (childSymbolDeclarationNode != nullptr) {
            throw std::runtime_error("not impl");
        }

        // does it have sub-statements?
        statement->visitBodies([state](BodyNode& body) -> bool {
            assignLocalVariableIndicesInBody(&body, state);
            return true;
        });
    }
}

static void assignLocalVariableIndices(ProcedureNode* procedure) {
    VariableIndexingState state{};
    assignLocalVariableIndicesInBody(procedure->body.get(), &state);
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
    assignLocalVariableIndices(procedureNode.get());
    auto pcode = emit(*procedureNode);
    pcode.push_back(static_cast<uint8_t>(vm::Opcode::kExit));

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
