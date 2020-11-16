#pragma once

#include "common.h"
#include "Object.h"
#include "ProcedureArtifact.h"
#include "ProgramMember.h"

namespace vm {

class Procedure : public ProgramMember {
   public:
    bool isSystemProcedure;
    std::optional<std::unique_ptr<ProcedureArtifact>> artifact;

    virtual ~Procedure();
    ProgramMemberType getProgramMemberType() const override;
};

}  // namespace vm
