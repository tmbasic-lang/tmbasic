#pragma once

#include "common.h"
#include "GlobalVariable.h"
#include "Procedure.h"

namespace vm {

class Program {
   public:
    void save(std::string filePath) const;

    std::vector<std::unique_ptr<Procedure>> procedures;
    std::vector<std::unique_ptr<GlobalVariable>> globalVariables;
};

}  // namespace vm
