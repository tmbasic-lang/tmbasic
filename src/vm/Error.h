#pragma once

#include "../common.h"

namespace vm {

// run build/scripts/updateErrors.sh to update BuiltInConstantList.cpp to match this enum
enum class ErrorCode {
    // Miscellaneous
    kValueNotPresent = 1,  // ERR_VALUE_NOT_PRESENT

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
};

class Error : public std::runtime_error {
   public:
    ErrorCode code;
    Error(ErrorCode code, std::string message);
    static Error fromDirectoryErrno(int posixError, const std::string& path);
    static Error fromFileErrno(int posixError, const std::string& filePath);
    static Error fromDirectoryWinError(uint32_t winError, const std::string& path);
};

}  // namespace vm
