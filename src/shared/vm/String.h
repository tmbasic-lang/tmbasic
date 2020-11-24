#pragma once

#include "../../common.h"
#include "shared/vm/Object.h"

namespace vm {

class String : public Object {
   public:
    const std::string value;
    String();
    explicit String(std::string value);
    String(const uint8_t* source, int length);
    ObjectType getObjectType() const override;
    size_t getHash() const override;
    bool equals(const Object& other) const override;
};

}  // namespace vm