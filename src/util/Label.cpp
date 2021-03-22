#include "../util/Label.h"
#include "../util/tvutil.h"

namespace util {

Label::Label(const TRect& r) : TLabel(r, "", nullptr) {}

Label::Label(TStringView text, TView* link) : TLabel(TRect(0, 0, static_cast<int>(cstrlen(text) + 3), 1), text, link) {}

void Label::setTitle(const std::string& title) {
    delete[] const_cast<char*>(text);  // NOLINT
    text = strdup(title.c_str());
}

}  // namespace util
