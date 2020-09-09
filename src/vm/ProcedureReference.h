#pragma once

#include "common.h"
#include "Object.h"

namespace vm {

class ProcedureReference : public Object {
    public:
    int cachedProcedureIndex;
    size_t signatureHash;
    std::string signature;
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}
