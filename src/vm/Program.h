#pragma once

#include "../common.h"
#include "vm/Object.h"
#include "vm/Procedure.h"
#include "vm/Value.h"

namespace vm {

class Program {
   public:
    std::vector<std::unique_ptr<Procedure>> procedures;
    std::vector<Value> globalValues;
    std::vector<boost::local_shared_ptr<Object>> globalObjects;
    void deserialize(const std::vector<uint8_t>& pcode);
    std::vector<uint8_t> serialize();
};

}  // namespace vm
