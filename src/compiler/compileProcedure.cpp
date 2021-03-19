#include "compiler/compileProcedure.h"
#include "compiler/bindNamedTypes.h"
#include "compiler/bindProcedureSymbols.h"
#include "compiler/parse.h"
#include "compiler/tokenize.h"
#include "compiler/typeCheck.h"
#include "util/cast.h"

using util::dynamic_cast_move;

namespace compiler {

CompilerResult compileProcedure(const SourceMember& sourceMember, CompiledProgram* compiledProgram) {
    auto tokens = tokenize(sourceMember.source, TokenizeType::kCompile);
    auto parserResult = parse(ParserRootProduction::kMember, tokens);
    if (!parserResult.isSuccess) {
        return CompilerResult::error(parserResult.message, *parserResult.token);
    }
    if (parserResult.node->getMemberType() != MemberType::kProcedure) {
        return CompilerResult::error("This member must be a subroutine or function.", tokens[0]);
    }
    auto procedureNode = dynamic_cast_move<ProcedureNode>(std::move(parserResult.node));

    auto compilerResult = bindProcedureSymbols(procedureNode.get(), *compiledProgram);
    if (!compilerResult.isSuccess) {
        return compilerResult;
    }

    compilerResult = bindNamedTypes(procedureNode.get(), *compiledProgram);
    if (!compilerResult.isSuccess) {
        return compilerResult;
    }

    compilerResult = typeCheck(procedureNode.get());
    if (!compilerResult.isSuccess) {
        return compilerResult;
    }

    return CompilerResult::success();
}

}  // namespace compiler
