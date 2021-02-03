#include "UTextPtr.h"

namespace util {

static std::unique_ptr<UText> createUText(const char* utf8) {
    auto status = U_ZERO_ERROR;
    auto utext = std::make_unique<UText>();
    *utext = UTEXT_INITIALIZER;
    utext_openUTF8(utext.get(), utf8, -1, &status);
    return utext;
}

UTextPtr::UTextPtr(const char* utf8) : _ptr(createUText(utf8)) {}

UTextPtr::~UTextPtr() {
    if (_ptr) {
        utext_close(_ptr.get());
    }
}

UText* UTextPtr::operator->() {
    return _ptr.get();
}

UText* UTextPtr::get() const {
    return _ptr.get();
}

UTextPtr::operator UText*() {
    return _ptr.get();
}

}  // namespace util
