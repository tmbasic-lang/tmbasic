#pragma once

#include "Procedure.h"
#include <vector>
#include <memory>

namespace vm {

class Program {
    std::vector<std::unique_ptr<Procedure>> procedures;
};

}  // namespace vm
