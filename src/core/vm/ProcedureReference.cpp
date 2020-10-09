#include "ProcedureReference.h"

using namespace vm;

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
    auto& otherProcedureReference = (const ProcedureReference&)other;
    return signature == otherProcedureReference.signature;
}
