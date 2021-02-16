#pragma once

#include "../common.h"

namespace util {

size_t getDecimalHash(const decimal::Decimal& x);
decimal::Decimal parseDecimalString(const std::string& str);
std::string decimalToString(const decimal::Decimal& x);

}  // namespace util
