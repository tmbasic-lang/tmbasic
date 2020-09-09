#pragma once

#include "common.h"
#include "Procedure.h"

namespace vm {

class Program {
    std::vector<boost::local_shared_ptr<Procedure>> procedures;
};

}  // namespace vm
