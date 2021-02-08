#pragma once

#include "../common.h"
#include "TargetPlatform.h"

namespace compiler {

std::vector<uint8_t> makeExeFile(const std::vector<uint8_t>& bytecode, TargetPlatform platform);

}  // namespace compiler
