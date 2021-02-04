#pragma once

#include "../../common.h"
#include "shared/vm/Object.h"

namespace vm {

class String : public Object {
   public:
    const icu::UnicodeString value;
    String();
    explicit String(const std::string& utf8);
    explicit String(icu::UnicodeString utf16);
    String(const uint8_t* source, int length);
    ObjectType getObjectType() const override;
    size_t getHash() const override;
    bool equals(const Object& other) const override;
    std::string toUtf8() const;
};

}  // namespace vm
