#include "compiler/compile.h"
#include "compiler/bindProcedureSymbols.h"
#include "compiler/parse.h"
#include "compiler/tokenize.h"
#include "compiler/typeCheck.h"
#include "util/cast.h"

using util::dynamic_cast_move;

namespace compiler {

static bool isCommentToken(const Token& x) {
    return x.type == TokenKind::kComment;
}

static void removeComments(std::vector<Token>* tokens) {
    tokens->erase(std::remove_if(tokens->begin(), tokens->end(), isCommentToken));
}

static void removeBlankLines(std::vector<Token>* tokens) {
    for (auto i = tokens->size() - 1; i >= 1; i--) {
        if ((*tokens)[i].type == TokenKind::kEndOfLine && (*tokens)[i - 1].type == TokenKind::kEndOfLine) {
            tokens->erase(tokens->begin() + i);
        }
    }
}

static CompilerResult compileGlobal(const SourceMember& /*sourceMember*/, CompiledProgram* /*compiledProgram*/) {
    return CompilerResult::success();
}

// precondition: all global variables must be compiled first
static CompilerResult compileProcedure(const SourceMember& sourceMember, CompiledProgram* compiledProgram) {
    auto tokens = tokenize(sourceMember.source);
    removeComments(&tokens);
    removeBlankLines(&tokens);

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

    compilerResult = typeCheck(procedureNode.get());
    if (!compilerResult.isSuccess) {
        return compilerResult;
    }

    return CompilerResult::success();
}

CompilerResult compile(const SourceMember& sourceMember, CompiledProgram* compiledProgram) {
    switch (sourceMember.memberType) {
        case SourceMemberType::kProcedure:
            return compileProcedure(sourceMember, compiledProgram);
        case SourceMemberType::kGlobal:
            return compileGlobal(sourceMember, compiledProgram);
        default:
            assert(false);
            throw std::runtime_error("Not implemented");
    }
}

}  // namespace compiler
