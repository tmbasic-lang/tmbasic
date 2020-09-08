#pragma once

#include "Type.h"
#include <optional>
#include <string>
#include <vector>

namespace vm {

class ProcedureArtifact {
   public:
    std::vector<uint8_t> instructions;
    std::string signature;
    std::optional<Type> returnType;
    std::vector<Type> parameterTypes;
};

}  // namespace vm
