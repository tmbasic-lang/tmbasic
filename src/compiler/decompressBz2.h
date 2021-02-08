#pragma once

#include "../common.h"

namespace compiler {

std::vector<uint8_t> decompressBz2(const uint8_t compressedData[], uint compressedLength);

}  // namespace compiler
