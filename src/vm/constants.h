#pragma once

#include "../common.h"

namespace vm {

// we store stack indices as 16-bit values in the pcode
const int kValueStackSize = 65535;
const int kObjectStackSize = 65535;

}  // namespace vm
