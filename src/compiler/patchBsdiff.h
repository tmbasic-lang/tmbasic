#pragma once

#include "../common.h"

namespace compiler {

std::vector<uint8_t> patchBsdiff(const uint8_t* original, uint originalLength, const uint8_t* diff, uint diffLength);

}  // namespace compiler
