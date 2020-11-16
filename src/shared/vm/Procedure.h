#pragma once

#include "../../common.h"
#include "shared/vm/Object.h"
#include "shared/vm/ProcedureArtifact.h"
#include "shared/vm/ProgramMember.h"

namespace vm {

class Procedure : public ProgramMember {
   public:
    bool isSystemProcedure;
    std::optional<std::unique_ptr<ProcedureArtifact>> artifact;

    virtual ~Procedure();
    ProgramMemberType getProgramMemberType() const override;
};

}  // namespace vm
