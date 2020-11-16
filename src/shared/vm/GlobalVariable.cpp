#include "GlobalVariable.h"

namespace vm {

GlobalVariable::~GlobalVariable() {}

ProgramMemberType GlobalVariable::getProgramMemberType() const {
    return ProgramMemberType::kGlobalVariable;
}

}  // namespace vm
