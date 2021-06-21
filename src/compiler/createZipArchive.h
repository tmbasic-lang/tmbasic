#pragma once

#include "../common.h"

namespace compiler {

class ZipEntry {
   public:
    std::string name;
    std::vector<uint8_t> data;
    ZipEntry(std::string name, std::vector<uint8_t> data);
};

void createZipArchive(const std::string& zipFilePath, const std::vector<ZipEntry>& entries);

}  // namespace compiler
