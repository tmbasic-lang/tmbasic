#pragma once

#include "../common.h"

namespace util {

extern const decimal::Decimal kDecimalNegativeOne;
extern const decimal::Decimal kDecimalZero;
extern const decimal::Decimal kDecimalOne;
extern const decimal::Decimal kDecimalTwo;

size_t getDecimalHash(const decimal::Decimal& x);
decimal::Decimal parseDecimalString(const std::string& str);
std::string decimalToString(const decimal::Decimal& x);
decimal::Decimal doubleToDecimal(double x);
double decimalToDouble(const decimal::Decimal& x);
decimal::Decimal round(const decimal::Decimal& x);

}  // namespace util
