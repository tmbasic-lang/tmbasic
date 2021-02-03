#pragma once

#include "../../common.h"

namespace util {

class UTextPtr {
   public:
    explicit UTextPtr(const char* utf8);
    ~UTextPtr();
    UText* operator->();
    UText* get() const;
    operator UText*();

   private:
    std::unique_ptr<UText> _ptr;
};

}  // namespace util
