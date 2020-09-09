#pragma once

#include "common.h"
#include "Procedure.h"

namespace vm {

class Program {
    std::vector<std::optional<std::unique_ptr<Procedure>>> procedures;
};

}  // namespace vm
