#pragma once

#include "../common.h"

namespace util {

std::string getFileName(const std::string& filePath);
std::string getFileNameWithoutExtension(const std::string& filePath);
std::string getExtension(const std::string& filePath);
std::string getDirectoryName(const std::string& filePath);
void createDirectory(const std::string& path);
std::string pathCombine(const std::string& dir, const std::string& name);

}  // namespace util
