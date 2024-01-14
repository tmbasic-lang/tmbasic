#pragma once

#include "../common.h"
#include "vm/Object.h"

namespace vm {

class String : public Object {
   public:
    const std::string value;  // UTF-8
    String();
    explicit String(std::string utf8);
    String(const uint8_t* utf8, int length);
    String(const char* utf8, int length);
    explicit String(const char* utf8);
    shared::ObjectType getObjectType() const override;
    size_t getHash() const override;
    bool equals(const Object& other) const override;
    const uint8_t* getUnistring() const;

   private:
    mutable std::optional<size_t> _hash{};  // lazy
};

}  // namespace vm
