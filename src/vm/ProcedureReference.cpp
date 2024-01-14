#include "ProcedureReference.h"

using shared::ObjectType;

namespace vm {

ObjectType ProcedureReference::getObjectType() const {
    return ObjectType::kProcedureReference;
}

size_t ProcedureReference::getHash() const {
    return std::hash<std::string>{}(signature);
}

bool ProcedureReference::equals(const Object& other) const {
    if (other.getObjectType() != ObjectType::kProcedureReference) {
        return false;
    }
    const auto& otherProcedureReference = dynamic_cast<const ProcedureReference&>(other);
    return signature == otherProcedureReference.signature;
}

}  // namespace vm
