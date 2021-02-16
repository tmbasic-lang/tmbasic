#pragma once

#include "../../common.h"
#include "shared/vm/Object.h"
#include "shared/vm/Procedure.h"
#include "shared/vm/Value.h"

namespace vm {

class Program {
   public:
    std::vector<std::unique_ptr<Procedure>> procedures;
    std::vector<Value> globalValues;
    std::vector<boost::local_shared_ptr<Object>> globalObjects;
};

}  // namespace vm
