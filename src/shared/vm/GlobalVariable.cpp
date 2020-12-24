#include "GlobalVariable.h"

namespace vm {

ProgramMemberType GlobalVariable::getProgramMemberType() const {
    return ProgramMemberType::kGlobalVariable;
}

}  // namespace vm
