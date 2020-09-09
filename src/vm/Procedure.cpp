#include "Procedure.h"

using namespace vm;

Kind Procedure::getKind() const {
    return Kind::kProcedure;
}

size_t Procedure::getHash() const {
    if (artifact.has_value()) {
        return artifact.value()->signatureHash;
    } else if (source.has_value()) {
        return std::hash<std::string>{}(source.value());
    } else {
        return 0;
    }
}

bool Procedure::equals(const Object& other) const {
    if (other.getKind() != Kind::kProcedure) {
        return false;
    }
    auto& otherProcedure = (const Procedure&)other;
    if (artifact.has_value()) {
        return otherProcedure.artifact.has_value() &&
            artifact.value()->signature == otherProcedure.artifact.value()->signature;
    } else if (source.has_value()) {
        return otherProcedure.source.has_value() &&
            source.value() == otherProcedure.source.value();
    } else {
        return !otherProcedure.source.has_value() && !otherProcedure.artifact.has_value();
    }
}
