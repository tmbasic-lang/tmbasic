#pragma once

#include "ProcedureArtifact.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace vm {

class Procedure {
   public:
    bool isSystemProcedure;
    std::optional<std::string> source;
    std::optional<std::unique_ptr<ProcedureArtifact>> artifact;
};

}  // namespace vm
