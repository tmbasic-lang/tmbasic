#include "Procedure.h"

namespace vm {

ProgramMemberType Procedure::getProgramMemberType() const {
    return ProgramMemberType::kProcedure;
}

}  // namespace vm
