#include "Compiler.h"
#include "core/util/cast.h"
#include "Scanner.h"
#include "bindProcedureSymbols.h"

using namespace basic;
using namespace vm;

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

static void removeComments(std::vector<Token>& tokens) {
    tokens.erase(std::remove_if(tokens.begin(), tokens.end(), isCommentToken));
}

static void removeBlankLines(std::vector<Token>& tokens) {
    for (auto i = tokens.size() - 1; i >= 1; i--) {
        if (tokens[i].type == TokenKind::kEndOfLine && tokens[i - 1].type == TokenKind::kEndOfLine) {
            tokens.erase(tokens.begin() + i);
        }
    }
}

CompilerResult Compiler::compileProcedure(Procedure& procedure, Program& program) {
    // source is missing if we're executing a precompiled program, which shouldn't run the compiler
    assert(procedure.source.has_value());

    auto tokens = Scanner::tokenize(*procedure.source);
    removeComments(tokens);
    removeBlankLines(tokens);

    auto parserResult = _parser.parseMember(tokens);
    if (!parserResult.isSuccess) {
        return CompilerResult::error(parserResult.message, *parserResult.token);
    } else if (parserResult.node->getMemberType() != MemberType::kProcedure) {
        return CompilerResult::error("This member must be a subroutine or function.", tokens[0]);
    }
    auto procedureNode = dynamic_cast_move<ProcedureNode>(std::move(parserResult.node));

    auto compilerResult = bindProcedureSymbols(*procedureNode, program);
    if (!compilerResult.isSuccess) {
        return compilerResult;
    }


    return CompilerResult::success();
}

}  // namespace compiler
