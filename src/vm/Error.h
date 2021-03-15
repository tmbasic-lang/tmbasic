#pragma once

#include "../common.h"

namespace vm {

enum class ErrorCode {
    kInvalidLocaleName = 1000,
    kValueNotPresent = 1001,
    kInvalidDateTime = 1002,
    kInvalidTimeZone = 1003,
};

class Error : public std::runtime_error {
   public:
    ErrorCode code;
    Error(ErrorCode code, const char* message);
};

}  // namespace vm
