#include "compileProcedures.h"
#include "CompilerException.h"
#include "bindProcedureSymbols.h"
#include "parse.h"
#include "tokenize.h"
#include "typeCheck.h"
#include "util/cast.h"
#include "vm/Opcode.h"

using util::dynamic_cast_move;

namespace compiler {

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

    auto vmProcedure = std::make_unique<vm::Procedure>();
    vmProcedure->instructions.push_back(static_cast<uint8_t>(vm::Opcode::kExit));
    compiledProgram->vmProgram.procedures.push_back(std::move(vmProcedure));
}

void compileProcedures(const SourceProgram& sourceProgram, CompiledProgram* compiledProgram) {
    for (const auto& member : sourceProgram.members) {
        if (member->memberType == SourceMemberType::kProcedure) {
            compileProcedure(*member, compiledProgram);
        }
    }
}

}  // namespace compiler
