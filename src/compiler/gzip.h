#pragma once

#include "../common.h"

namespace compiler {

std::vector<uint8_t> gzip(const std::vector<uint8_t>& src);
std::vector<uint8_t> gunzip(const std::vector<uint8_t>& src);

}  // namespace compiler
