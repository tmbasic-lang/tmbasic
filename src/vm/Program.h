#pragma once

#include "../common.h"
#include "vm/Object.h"
#include "vm/Procedure.h"
#include "vm/Value.h"

namespace vm {

class Program {
   public:
    size_t startupProcedureIndex = 0;
    std::vector<std::unique_ptr<Procedure>> procedures;
    std::vector<Value> globalValues;
    std::vector<boost::intrusive_ptr<Object>> globalObjects;
    void deserialize(const std::vector<uint8_t>& pcode);
};

}  // namespace vm
