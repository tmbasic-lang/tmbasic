#include "CompilerResult.h"

using namespace basic;

namespace compiler {

CompilerResult::CompilerResult(bool isSuccess, std::string message, Token token)
    : isSuccess(isSuccess), message(std::move(message)), token(token) {}

CompilerResult CompilerResult::success() {
    return CompilerResult(true, std::string(), Token(0, 0, TokenKind::kEndOfFile, std::string()));
}

CompilerResult CompilerResult::error(std::string message, Token token) {
    return CompilerResult(false, std::move(message), token);
}

}  // namespace compiler
