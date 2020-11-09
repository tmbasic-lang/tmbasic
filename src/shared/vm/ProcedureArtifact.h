#pragma once

#include "common.h"
#include "shared/basic/ast.h"

namespace vm {

class ProcedureArtifact {
   public:
    std::vector<uint8_t> instructions;
    std::string signature;
    size_t signatureHash;
    std::optional<std::unique_ptr<basic::TypeNode>> returnType;
    std::vector<std::unique_ptr<basic::TypeNode>> parameterTypes;
    int numValueParameters;
    int numObjectParameters;
};

}  // namespace vm
