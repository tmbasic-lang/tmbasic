#pragma once

#include "common.h"
#include "Object.h"
#include "Value.h"

namespace vm {

class GlobalVariable {
   public:
    std::string name;           // used for display only
    std::string lowercaseName;  // used for symbol binding
    std::optional<std::string> source;
    Value value;
    boost::local_shared_ptr<Object> object;
};

}  // namespace vm
