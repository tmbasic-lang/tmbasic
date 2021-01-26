#pragma once

#include "../common.h"

namespace tmbasic {

bool tryParseInt(const std::string& text, int* output);
int parseUserInt(const char* text, const char* fieldName, int minValue, int maxValue);

}  // namespace tmbasic
