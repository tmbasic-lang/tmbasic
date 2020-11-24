#include "compiler/compile.h"
#include "compiler/bindProcedureSymbols.h"
#include "compiler/parse.h"
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

CompilerResult compile(Procedure* procedure, Program* program) {
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
