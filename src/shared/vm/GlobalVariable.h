#pragma once

#include "common.h"
#include "Object.h"
#include "ProgramMember.h"
#include "Value.h"

namespace vm {

class GlobalVariable : public ProgramMember {
   public:
    std::string lowercaseName;  // used for symbol binding
    Value value;
    boost::local_shared_ptr<Object> object;

    virtual ~GlobalVariable();
    ProgramMemberType getProgramMemberType() const override;
};

}  // namespace vm
