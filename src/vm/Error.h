#pragma once

#include "../common.h"

namespace vm {

// run build/scripts/updateErrors.sh to update BuiltInConstantList.cpp to match this enum
enum class ErrorCode {
    // General purpose
    kInvalidArgument = 50,  // ERR_INVALID_ARGUMENT

    // I/O
    kIoFailure = 100,           // ERR_IO_FAILURE
    kFileNotFound = 101,        // ERR_FILE_NOT_FOUND
    kAccessDenied = 102,        // ERR_ACCESS_DENIED
    kPathTooLong = 103,         // ERR_PATH_TOO_LONG
    kDiskFull = 104,            // ERR_DISK_FULL
    kPathIsDirectory = 105,     // ERR_PATH_IS_DIRECTORY
    kPathIsNotDirectory = 106,  // ERR_PATH_IS_NOT_DIRECTORY
    kDirectoryNotEmpty = 107,   // ERR_DIRECTORY_NOT_EMPTY

    // ICU
    kInternalIcuError = 200,   // ERR_INTERNAL_ICU_ERROR
    kInvalidDateTime = 201,    // ERR_INVALID_DATETIME
    kInvalidTimeZone = 202,    // ERR_INVALID_TIMEZONE
    kInvalidLocaleName = 203,  // ERR_INVALID_LOCALE_NAME

    // Collections
    kMapKeyNotFound = 300,         // ERR_MAP_KEY_NOT_FOUND
    kListIndexOutOfRange = 301,    // ERR_LIST_INDEX_OUT_OF_RANGE
    kValueNotPresent = 302,        // ERR_VALUE_NOT_PRESENT
    kInternalTypeConfusion = 303,  // ERR_INTERNAL_TYPE_CONFUSION
    kListIsEmpty = 304,            // ERR_LIST_IS_EMPTY
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
