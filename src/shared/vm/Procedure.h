#pragma once

#include "../../common.h"
#include "shared/vm/Object.h"
#include "shared/vm/ProgramMember.h"

namespace vm {

class Procedure : public ProgramMember {
   public:
    std::vector<uint8_t> instructions;
    ProgramMemberType getProgramMemberType() const override;
};

}  // namespace vm
