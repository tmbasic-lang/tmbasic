#pragma once

#include "common.h"
#include "Type.h"

namespace vm {

class ProcedureArtifact {
   public:
    std::vector<uint8_t> instructions;
    std::string signature;
    size_t signatureHash;
    std::optional<Type> returnType;
    std::vector<Type> parameterTypes;
    int numValueParameters;
    int numObjectParameters;
};

}  // namespace vm
