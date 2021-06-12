#pragma once

#include "../common.h"

namespace util {

// tvision will take ownership when we add it to its parent
template <typename T>
class ViewPtr {
   public:
    template <typename... Args>
    explicit ViewPtr(Args&&... args) : _ptr(new T(std::forward<Args>(args)...)) {}
    ViewPtr(const ViewPtr& x) = delete;
    ViewPtr(ViewPtr& x) : _ptr(x.take()) {}
    ViewPtr(ViewPtr&& x) : _ptr(x.take()) {}

    ~ViewPtr() {
        if (_owned) {
            delete _ptr;
            _ptr = nullptr;
        }
    }

    T* operator->() { return _ptr; }

    operator T*() { return _ptr; }

    void addTo(TGroup* parent) {
        assert(_ptr != nullptr);
        parent->insert(_ptr);  // ownership transfer
        _owned = false;
    }

    gsl::owner<T*> take() {
        _owned = false;
        return _ptr;
    }

   private:
    T* _ptr;
    bool _owned = true;
};

}  // namespace util
