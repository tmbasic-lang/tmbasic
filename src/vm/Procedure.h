#pragma once

#include "../common.h"
#include "vm/Object.h"

namespace vm {

class Procedure {
   public:
    std::vector<uint8_t> instructions;
};

}  // namespace vm
