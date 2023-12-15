#pragma once

#include "../common.h"

namespace vm {

// run build/scripts/updateErrors.sh to update BuiltInConstantList.cpp to match this enum
enum class ErrorCode {
    // General purpose
    kInvalidArgument = 50,   // ERR_INVALID_ARGUMENT
    kInvalidOperation = 51,  // ERR_INVALID_OPERATION

    // I/O
    kIoFailure = 100,           // ERR_IO_FAILURE
    kFileNotFound = 101,        // ERR_FILE_NOT_FOUND
    kAccessDenied = 102,        // ERR_ACCESS_DENIED
    kPathTooLong = 103,         // ERR_PATH_TOO_LONG
    kDiskFull = 104,            // ERR_DISK_FULL
    kPathIsDirectory = 105,     // ERR_PATH_IS_DIRECTORY
    kPathIsNotDirectory = 106,  // ERR_PATH_IS_NOT_DIRECTORY
    kDirectoryNotEmpty = 107,   // ERR_DIRECTORY_NOT_EMPTY
    kInvalidPath = 108,         // ERR_INVALID_PATH

    // Strings
    kInternalUnicodeError = 200,     // ERR_INTERNAL_UNICODE_ERROR
    kInvalidUnicodeCodePoint = 201,  // ERR_INVALID_UNICODE_CODE_POINT
    kInvalidNumberFormat = 202,      // ERR_INVALID_NUMBER_FORMAT

    // Collections
    kMapKeyNotFound = 300,         // ERR_MAP_KEY_NOT_FOUND
    kListIndexOutOfRange = 301,    // ERR_LIST_INDEX_OUT_OF_RANGE
    kValueNotPresent = 302,        // ERR_VALUE_NOT_PRESENT
    kInternalTypeConfusion = 303,  // ERR_INTERNAL_TYPE_CONFUSION
    kListIsEmpty = 304,            // ERR_LIST_IS_EMPTY

    // Forms
    kFormNotFound = 400,        // ERR_FORM_NOT_FOUND
    kControlNotFound = 401,     // ERR_CONTROL_NOT_FOUND
    kInvalidControlType = 402,  // ERR_INVALID_CONTROL_TYPE

    // Dates/times
    kInvalidDateTime = 500,  // ERR_INVALID_DATETIME
    kInvalidTimeZone = 501,  // ERR_INVALID_TIMEZONE
};

class Error : public std::runtime_error {
   public:
    ErrorCode code;
    Error(ErrorCode code, const std::string& message);
    static Error fromDirectoryErrno(int posixError, const std::string& path);
    static Error fromFileErrno(int posixError, const std::string& filePath);
    static Error fromDirectoryWinError(uint32_t winError, const std::string& path);
};

}  // namespace vm
