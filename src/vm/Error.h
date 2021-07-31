#pragma once

#include "../common.h"

namespace vm {

// note: update BuiltInConstantList.cpp to match this list
enum class ErrorCode {
    kInvalidLocaleName = 1000,
    kValueNotPresent = 1001,
    kInvalidDateTime = 1002,
    kInvalidTimeZone = 1003,
    kInternalIcuError = 1004,
};

class Error : public std::runtime_error {
   public:
    ErrorCode code;
    inline Error(ErrorCode code, std::string message) : code(code), std::runtime_error(std::move(message)) {}
};

}  // namespace vm
