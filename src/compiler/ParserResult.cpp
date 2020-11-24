#include "ParserResult.h"

using basic::Node;
using basic::Token;

namespace compiler {

ParserResult::ParserResult(std::string message, Token token)
    : isSuccess(false), message(std::move(message)), token(token) {}

ParserResult::ParserResult(std::unique_ptr<Node> node) : isSuccess(true), node(std::move(node)) {}

}  // namespace compiler
