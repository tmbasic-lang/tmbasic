#pragma once

#include "../common.h"

namespace util {

std::string getFileName(const std::string& filePath);
std::string getDirectoryName(const std::string& filePath);
void createDirectory(const std::string& path);

}  // namespace util
