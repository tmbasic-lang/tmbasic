#include "Procedure.h"

namespace vm {

Procedure::~Procedure() {}

ProgramMemberType Procedure::getProgramMemberType() const {
    return ProgramMemberType::kProcedure;
}

}  // namespace vm
