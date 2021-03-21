#pragma once

#include "../common.h"

namespace util {

// tvision will take ownership when we add it to the desktop
template <typename T>
class WindowPtr {
   public:
    template <typename... Args>
    explicit WindowPtr(Args&&... args) {
        TProgram::deskTop->insert(_ptr = new T(std::forward<Args>(args)...));
    }

    T* get() const { return _ptr; }

   private:
    T* _ptr;
};

}  // namespace util
