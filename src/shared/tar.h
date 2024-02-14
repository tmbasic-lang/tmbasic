#pragma once

#include "../common.h"

namespace shared {

class TarEntry {
   public:
    std::string name;
    std::vector<uint8_t> data;
    uint mode;
    TarEntry(std::string name, std::vector<uint8_t> data, uint mode);
};

std::vector<uint8_t> tar(const std::vector<TarEntry>& entries);
void untar(
    const char* tarData,
    size_t tarSize,
    const std::function<void(const std::string& name, std::vector<char> data)>& entryCallback);

}  // namespace shared
