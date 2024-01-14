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
    addError("ERR_INVALID_ARGUMENT", vm::ErrorCode::kInvalidArgument);
    addError("ERR_INVALID_OPERATION", vm::ErrorCode::kInvalidOperation);
    addError("ERR_IO_FAILURE", vm::ErrorCode::kIoFailure);
    addError("ERR_FILE_NOT_FOUND", vm::ErrorCode::kFileNotFound);
    addError("ERR_ACCESS_DENIED", vm::ErrorCode::kAccessDenied);
    addError("ERR_PATH_TOO_LONG", vm::ErrorCode::kPathTooLong);
    addError("ERR_DISK_FULL", vm::ErrorCode::kDiskFull);
    addError("ERR_PATH_IS_DIRECTORY", vm::ErrorCode::kPathIsDirectory);
    addError("ERR_PATH_IS_NOT_DIRECTORY", vm::ErrorCode::kPathIsNotDirectory);
    addError("ERR_DIRECTORY_NOT_EMPTY", vm::ErrorCode::kDirectoryNotEmpty);
    addError("ERR_INVALID_PATH", vm::ErrorCode::kInvalidPath);
    addError("ERR_INTERNAL_UNICODE_ERROR", vm::ErrorCode::kInternalUnicodeError);
    addError("ERR_INVALID_UNICODE_CODE_POINT", vm::ErrorCode::kInvalidUnicodeCodePoint);
    addError("ERR_INVALID_NUMBER_FORMAT", vm::ErrorCode::kInvalidNumberFormat);
    addError("ERR_MAP_KEY_NOT_FOUND", vm::ErrorCode::kMapKeyNotFound);
    addError("ERR_LIST_INDEX_OUT_OF_RANGE", vm::ErrorCode::kListIndexOutOfRange);
    addError("ERR_VALUE_NOT_PRESENT", vm::ErrorCode::kValueNotPresent);
    addError("ERR_INTERNAL_TYPE_CONFUSION", vm::ErrorCode::kInternalTypeConfusion);
    addError("ERR_LIST_IS_EMPTY", vm::ErrorCode::kListIsEmpty);
    addError("ERR_FORM_NOT_FOUND", vm::ErrorCode::kFormNotFound);
    addError("ERR_CONTROL_NOT_FOUND", vm::ErrorCode::kControlNotFound);
    addError("ERR_INVALID_CONTROL_TYPE", vm::ErrorCode::kInvalidControlType);
    addError("ERR_INVALID_DATETIME", vm::ErrorCode::kInvalidDateTime);
    addError("ERR_INVALID_TIMEZONE", vm::ErrorCode::kInvalidTimeZone);
    // end_errors
}

void BuiltInConstantList::addError(const std::string& name, vm::ErrorCode code) {
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
