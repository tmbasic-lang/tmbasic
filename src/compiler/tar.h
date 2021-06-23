#pragma once

#include "../common.h"

namespace compiler {

class TarEntry {
   public:
    std::string name;
    std::vector<uint8_t> data;
    uint mode;
    TarEntry(std::string name, std::vector<uint8_t> data, uint mode);
};

std::vector<uint8_t> tar(const std::vector<TarEntry>& entries);

}  // namespace compiler
