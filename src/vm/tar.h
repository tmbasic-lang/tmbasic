#pragma once

#include "../common.h"

namespace vm {

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
    std::function<void(const char* name, const char* data, size_t length)> entryCallback);

}  // namespace vm
