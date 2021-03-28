#include "../util/Label.h"
#include "../util/tvutil.h"

namespace util {

Label::Label(const TRect& r) : TLabel(r, "", nullptr) {}

Label::Label(TStringView text, TView* link) : TLabel(TRect(0, 0, static_cast<int>(cstrlen(text) + 3), 1), text, link) {}

void Label::setTitle(const std::string& title) {
    delete[] const_cast<char*>(text);  // NOLINT
    text = strdup(title.c_str());
}

TColorAttr Label::mapColor(uchar index) {
    switch (index) {
        case 1:
            if (colorActive.has_value()) {
                return *colorActive;
            }
            break;
        case 2:
            if (colorInactive.has_value()) {
                return *colorInactive;
            }
            break;
        case 3:
            if (colorFocused.has_value()) {
                return *colorFocused;
            }
            break;
        case 4:
            if (colorSelected.has_value()) {
                return *colorSelected;
            }
            break;
        case 5:
            if (colorDivider.has_value()) {
                return *colorDivider;
            }
            break;
        default:
            break;
    }

    return TLabel::mapColor(index);
}

}  // namespace util
