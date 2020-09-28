#include "ProcedureReference.h"

using namespace vm;

Kind ProcedureReference::getKind() const {
    return Kind::kProcedureReference;
}

size_t ProcedureReference::getHash() const {
    return std::hash<std::string>{}(signature);
}

bool ProcedureReference::equals(const Object& other) const {
    if (other.getKind() != Kind::kProcedureReference) {
        return false;
    }
    auto& otherProcedureReference = (const ProcedureReference&)other;
    return signature == otherProcedureReference.signature;
}
