#pragma once

#include "common.h"
#include "Procedure.h"

namespace vm {

class Program {
   public:
    std::vector<std::unique_ptr<Procedure>> procedures;
};

}  // namespace vm
