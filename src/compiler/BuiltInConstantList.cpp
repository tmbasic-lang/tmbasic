#include "BuiltInConstantList.h"
#include "../vm/constants.h"

namespace compiler {

BuiltInConstantList::BuiltInConstantList()
    : _numberType(boost::make_local_shared<TypeNode>(Kind::kNumber, Token{})),
      _stringType(boost::make_local_shared<TypeNode>(Kind::kString, Token{})) {
    addNumber("PI", shared::parseDecimalString("3.141592653589793238462643383279502"));
    addNumber("EULER", shared::parseDecimalString("2.718281828459045235360287471352662"));

    // this is updated automatically by build/scripts/updateErrors.sh
    // start_errors
    addError("ERR_INVALID_ARGUMENT", shared::ErrorCode::kInvalidArgument);
    addError("ERR_INVALID_OPERATION", shared::ErrorCode::kInvalidOperation);
    addError("ERR_IO_FAILURE", shared::ErrorCode::kIoFailure);
    addError("ERR_FILE_NOT_FOUND", shared::ErrorCode::kFileNotFound);
    addError("ERR_ACCESS_DENIED", shared::ErrorCode::kAccessDenied);
    addError("ERR_PATH_TOO_LONG", shared::ErrorCode::kPathTooLong);
    addError("ERR_DISK_FULL", shared::ErrorCode::kDiskFull);
    addError("ERR_PATH_IS_DIRECTORY", shared::ErrorCode::kPathIsDirectory);
    addError("ERR_PATH_IS_NOT_DIRECTORY", shared::ErrorCode::kPathIsNotDirectory);
    addError("ERR_DIRECTORY_NOT_EMPTY", shared::ErrorCode::kDirectoryNotEmpty);
    addError("ERR_INVALID_PATH", shared::ErrorCode::kInvalidPath);
    addError("ERR_INTERNAL_UNICODE_ERROR", shared::ErrorCode::kInternalUnicodeError);
    addError("ERR_INVALID_UNICODE_CODE_POINT", shared::ErrorCode::kInvalidUnicodeCodePoint);
    addError("ERR_INVALID_NUMBER_FORMAT", shared::ErrorCode::kInvalidNumberFormat);
    addError("ERR_MAP_KEY_NOT_FOUND", shared::ErrorCode::kMapKeyNotFound);
    addError("ERR_LIST_INDEX_OUT_OF_RANGE", shared::ErrorCode::kListIndexOutOfRange);
    addError("ERR_VALUE_NOT_PRESENT", shared::ErrorCode::kValueNotPresent);
    addError("ERR_INTERNAL_TYPE_CONFUSION", shared::ErrorCode::kInternalTypeConfusion);
    addError("ERR_LIST_IS_EMPTY", shared::ErrorCode::kListIsEmpty);
    addError("ERR_FORM_NOT_FOUND", shared::ErrorCode::kFormNotFound);
    addError("ERR_CONTROL_NOT_FOUND", shared::ErrorCode::kControlNotFound);
    addError("ERR_INVALID_CONTROL_TYPE", shared::ErrorCode::kInvalidControlType);
    addError("ERR_INVALID_DATETIME", shared::ErrorCode::kInvalidDateTime);
    addError("ERR_INVALID_TIMEZONE", shared::ErrorCode::kInvalidTimeZone);
    // end_errors
}

void BuiltInConstantList::addError(const std::string& name, shared::ErrorCode code) {
    addNumber(name, static_cast<int64_t>(code));
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
