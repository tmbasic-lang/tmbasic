#include "compiler/compile.h"
#include "compiler/bindProcedureSymbols.h"
#include "compiler/tokenize.h"
#include "shared/util/cast.h"

using basic::MemberType;
using basic::ProcedureNode;
using basic::Token;
using basic::TokenKind;
using util::dynamic_cast_move;
using vm::Procedure;
using vm::Program;

namespace compiler {

CompilerResult::CompilerResult(bool isSuccess, std::string message, basic::Token token)
    : isSuccess(isSuccess), message(std::move(message)), token(token) {}

CompilerResult CompilerResult::success() {
    return CompilerResult(true, std::string(), basic::Token(0, 0, basic::TokenKind::kEndOfFile, std::string()));
}

CompilerResult CompilerResult::error(std::string message, basic::Token token) {
    return CompilerResult(false, std::move(message), token);
}

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

CompilerResult compileProcedure(Procedure* procedure, Program* program) {
    // source is missing if we're executing a precompiled program, which shouldn't run the compiler
    assert(procedure->source.has_value());

    auto tokens = tokenize(*procedure->source);
    removeComments(&tokens);
    removeBlankLines(&tokens);

    auto parserResult = parse(ParserRootProduction::kMember, tokens);
    if (!parserResult.isSuccess) {
        return CompilerResult::error(parserResult.message, *parserResult.token);
    } else if (parserResult.node->getMemberType() != MemberType::kProcedure) {
        return CompilerResult::error("This member must be a subroutine or function.", tokens[0]);
    }
    auto procedureNode = dynamic_cast_move<ProcedureNode>(std::move(parserResult.node));

    auto compilerResult = bindProcedureSymbols(procedureNode.get(), *program);
    if (!compilerResult.isSuccess) {
        return compilerResult;
    }

    return CompilerResult::success();
}

}  // namespace compiler
