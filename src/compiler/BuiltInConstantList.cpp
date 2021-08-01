#include "BuiltInConstantList.h"

using decimal::Decimal;

namespace compiler {

BuiltInConstantList::BuiltInConstantList()
    : _numberType(boost::make_local_shared<TypeNode>(Kind::kNumber, Token{})),
      _stringType(boost::make_local_shared<TypeNode>(Kind::kString, Token{})) {
    addNumber("PI", util::parseDecimalString("3.141592653589793238462643383279502"));
    addNumber("EULER", util::parseDecimalString("2.718281828459045235360287471352662"));

    // this is updated automatically by build/scripts/updateErrors.sh
    // start_errors
    addError("ERR_VALUE_NOT_PRESENT", vm::ErrorCode::kValueNotPresent);
    addError("ERR_IO_FAILURE", vm::ErrorCode::kIoFailure);
    addError("ERR_FILE_NOT_FOUND", vm::ErrorCode::kFileNotFound);
    addError("ERR_ACCESS_DENIED", vm::ErrorCode::kAccessDenied);
    addError("ERR_PATH_TOO_LONG", vm::ErrorCode::kPathTooLong);
    addError("ERR_DISK_FULL", vm::ErrorCode::kDiskFull);
    addError("ERR_PATH_IS_DIRECTORY", vm::ErrorCode::kPathIsDirectory);
    addError("ERR_INTERNAL_ICU_ERROR", vm::ErrorCode::kInternalIcuError);
    addError("ERR_INVALID_DATETIME", vm::ErrorCode::kInvalidDateTime);
    addError("ERR_INVALID_TIMEZONE", vm::ErrorCode::kInvalidTimeZone);
    addError("ERR_INVALID_LOCALE_NAME", vm::ErrorCode::kInvalidLocaleName);
    // end_errors
}

void BuiltInConstantList::addError(std::string name, vm::ErrorCode code) {
    addNumber(std::move(name), static_cast<int64_t>(code));
}

void BuiltInConstantList::addNumber(const std::string& name, const decimal::Decimal& number) {
    auto node = std::make_unique<ConstStatementNode>(
        name, std::make_unique<LiteralNumberExpressionNode>(number, Token{}), Token{});
    node->evaluatedType = _numberType;
    constants.insert(std::pair(name, std::move(node)));
}

void BuiltInConstantList::addString(const std::string& name, std::string string) {
    auto node = std::make_unique<ConstStatementNode>(
        name, std::make_unique<LiteralStringExpressionNode>(std::move(string), Token{}), Token{});
    node->evaluatedType = _stringType;
    constants.insert(std::pair(name, std::move(node)));
}

}  // namespace compiler
