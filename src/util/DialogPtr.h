#pragma once

#include "../common.h"

namespace util {

template <typename T>
class DialogPtr {
   public:
    template <typename... Args>
    explicit DialogPtr(Args&&... args) : _ptr(std::make_unique<T>(std::forward<Args>(args)...)) {}

    ~DialogPtr() {
        if (_ptr) {
            _ptr->shutDown();
        }
    }

    T* operator->() { return _ptr.get(); }

    T* get() const { return _ptr.get(); }

    operator T*() { return _ptr.get(); }

   private:
    std::unique_ptr<T> _ptr;
};

}  // namespace util
