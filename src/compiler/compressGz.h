#pragma once

#include "../common.h"

namespace compiler {

std::vector<uint8_t> compressGz(const std::vector<uint8_t>& src);

}  // namespace compiler
