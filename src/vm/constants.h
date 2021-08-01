#pragma once

#include "../common.h"

namespace vm {

// we store stack indices as 16-bit values in the pcode
const int kValueStackSize = 65535;
const int kObjectStackSize = 65535;

#ifdef _WIN32
const std::string kNewLine = "\r\n";
#else
const std::string kNewLine = "\n";
#endif

}  // namespace vm
