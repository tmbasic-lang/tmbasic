#pragma once

#include "../common.h"

namespace shared {

std::string to_lower_copy(const std::string& str);
std::string trim_copy(const std::string& str);
void replace_all(std::string* haystack, const std::string& needle, const std::string& replacement);
std::string replace_all_copy(const std::string& haystack, const std::string& needle, const std::string& replacement);

}  // namespace shared
