#pragma once

#include "../common.h"
#include "vm/Object.h"

namespace vm {

class String : public Object {
   public:
    const icu::UnicodeString value;
    String();
    explicit String(const std::string& utf8);
    explicit String(icu::UnicodeString utf16);
    String(const uint8_t* utf8, int length);
    String(const char* utf8, int length);
    String(const char* utf8);
    ObjectType getObjectType() const override;
    size_t getHash() const override;
    bool equals(const Object& other) const override;
    std::string toUtf8() const;
};

}  // namespace vm
