#pragma once

#include "../common.h"
#include "compiler/Token.h"

namespace compiler {

enum class CompilerErrorCode {
    kInternal,
    kSyntax,
    kDuplicateSymbolName,
    kSymbolNotFound,
    kInvalidGlobalVariableType,
    kWrongMemberType,
    kMissingMainSub,
    kDuplicateTypeName,
    kTypeNotFound,
    kTooManyLiteralListElements,
    kTooManyCallArguments,
    kTooManyLocalVariables,
    kInputTargetNotVariableName,
    kSubCalledAsFunction,
    kProcedureNotFound,
    kTypeMismatch,
    kInvalidListIndex,
    kEmptyLiteralList,
    kInvalidAssignmentTarget,
    kMultipleSelectCaseDefaults,
    kYieldOutsideDimCollection,
    kNoYieldsInDimCollection,
    kInvalidYieldType,
    kContinueOutsideLoop,
    kContinueTypeMismatch,
    kExitOutsideLoop,
    kExitTypeMismatch,
    kFieldNotFound,
    kTooManyIndexArguments,
    kInvalidTypeConversion,
    kInvalidReturn,
    kControlReachesEndOfFunction,
    kRecursiveRecordType,
};

class CompilerException : public std::exception {
   public:
    CompilerErrorCode code;
    std::string message;
    Token token;
    inline CompilerException(CompilerErrorCode code, const std::string& message, const Token& token)
        : code(code), message(message), token(token) {}
};

}  // namespace compiler
