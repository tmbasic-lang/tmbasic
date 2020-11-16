#pragma once

#include "common.h"
#include "GlobalVariable.h"
#include "Procedure.h"

namespace vm {

class Program {
   public:
    std::vector<std::unique_ptr<Procedure>> procedures;
    std::vector<std::unique_ptr<GlobalVariable>> globalVariables;
};

}  // namespace vm
