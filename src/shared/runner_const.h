#pragma once

#include "../common.h"

namespace shared {

// The last 4 bytes of the runner exe file are this sentinel value.
constexpr uint32_t kPcodeSentinel = 0x3b35ade6;

}  // namespace shared
