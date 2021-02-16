#include "CompilerResult.h"

namespace compiler {

CompilerResult::CompilerResult(bool isSuccess, std::string message, Token token)
    : isSuccess(isSuccess), message(std::move(message)), token(std::move(token)) {}

CompilerResult CompilerResult::success() {
    return CompilerResult(true, std::string(), Token(0, 0, TokenKind::kEndOfFile, std::string()));
}

CompilerResult CompilerResult::error(std::string message, Token token) {
    return CompilerResult(false, std::move(message), std::move(token));
}

}  // namespace compiler
