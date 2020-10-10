#pragma once

#include "common.h"

namespace util {

std::decimal::decimal64 parseDecimalString(const std::string& str);
std::string decimalToString(std::decimal::decimal64 x);

}  // namespace util
