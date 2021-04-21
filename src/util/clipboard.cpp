#include "clipboard.h"
#include <libclipboard.h>

namespace util {

static std::string _fallbackClipboardText{};

class ClipboardPtr {
   public:
    ClipboardPtr() : clipboard(clipboard_new(nullptr)) {}
    ClipboardPtr(const ClipboardPtr&) = delete;
    ClipboardPtr& operator=(const ClipboardPtr&) = delete;
    ClipboardPtr(ClipboardPtr&&) = delete;
    ClipboardPtr& operator=(ClipboardPtr&&) = delete;
    ~ClipboardPtr() {
        if (clipboard != nullptr) {
            clipboard_free(clipboard);
        }
    }
    clipboard_c* get() const { return clipboard; }
    bool valid() const { return clipboard != nullptr; }
    clipboard_c* clipboard;
};

struct CStringFree {
    void operator()(gsl::owner<char*> str) {
        free(str);  // NOLINT(cppcoreguidelines-no-malloc)
    }
};

std::string getClipboard() {
    ClipboardPtr clipboard{};
    if (clipboard.valid()) {
        std::unique_ptr<char, CStringFree> buf{ clipboard_text(clipboard.get()) };
        return buf.get();
    }
    return _fallbackClipboardText;
}

void setClipboard(const std::string& text) {
    ClipboardPtr clipboard{};
    if (clipboard.valid()) {
        clipboard_set_text(clipboard.get(), text.c_str());
    }
    _fallbackClipboardText = text;
}

}  // namespace util
