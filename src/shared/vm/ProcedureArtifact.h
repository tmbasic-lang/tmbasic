#pragma once

#include "../../common.h"
#include "shared/basic/ast.h"

namespace vm {

class ProcedureArtifact {
   public:
    // in a published build of the user's program, only instructions will be present
    std::vector<uint8_t> instructions;

    // the rest of these fields are present only in internal (debug) builds of the user's program
    std::string signature = "";
    size_t signatureHash = 0;
    std::optional<std::unique_ptr<basic::TypeNode>> returnType;
    std::vector<std::unique_ptr<basic::TypeNode>> parameterTypes;
    int numValueParameters = 0;
    int numObjectParameters = 0;
};

}  // namespace vm
