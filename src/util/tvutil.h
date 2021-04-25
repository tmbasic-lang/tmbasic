#pragma once

#include "../common.h"

namespace util {

bool tryParseInt(const std::string& text, int* output);
int parseUserInt(const char* text, const char* fieldName, int minValue, int maxValue);
void validateIdentifier(const std::string& text, const char* fieldName);
std::string ellipsis(const std::string& text, size_t maxLen);

}  // namespace util
