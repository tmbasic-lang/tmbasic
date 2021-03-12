#pragma once

#include "../common.h"

namespace vm {

enum class ErrorCode {
    kInvalidLocaleName = 1000,
    kValueNotPresent = 1001,
    kInvalidDateTime = 1002,
};

}  // namespace vm
