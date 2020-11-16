#pragma once

#include "../../common.h"
#include "shared/basic/ast.h"
#include "shared/vm/Object.h"

namespace vm {

class ProcedureReference : public Object {
   public:
    int cachedProcedureIndex;
    size_t signatureHash;
    std::string signature;
    ObjectType getObjectType() const override;
    size_t getHash() const override;
    bool equals(const Object& other) const override;
};

}  // namespace vm
