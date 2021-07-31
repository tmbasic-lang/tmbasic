#include "BuiltInConstantList.h"

using decimal::Decimal;

namespace compiler {

BuiltInConstantList::BuiltInConstantList()
    : _numberType(boost::make_local_shared<TypeNode>(Kind::kNumber, Token{})),
      _stringType(boost::make_local_shared<TypeNode>(Kind::kString, Token{})) {
    addNumber("PI", util::parseDecimalString("3.141592653589793238462643383279502"));
}

void BuiltInConstantList::addNumber(std::string name, decimal::Decimal number) {
    auto node = std::make_unique<ConstStatementNode>(
        name, std::make_unique<LiteralNumberExpressionNode>(number, Token{}), Token{});
    node->evaluatedType = _numberType;
    constants.insert(std::pair(name, std::move(node)));
}

void BuiltInConstantList::addString(std::string name, std::string string) {
    auto node = std::make_unique<ConstStatementNode>(
        name, std::make_unique<LiteralStringExpressionNode>(std::move(string), Token{}), Token{});
    node->evaluatedType = _stringType;
    constants.insert(std::pair(name, std::move(node)));
}

}  // namespace compiler
