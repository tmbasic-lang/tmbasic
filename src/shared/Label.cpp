#include "../shared/Label.h"
#include "../shared/tvutil.h"

namespace shared {

Label::Label(const TRect& r) : TLabel(r, "", nullptr) {}

Label::Label(TStringView text, TView* link) : TLabel(TRect(0, 0, (cstrlen(text) + 3), 1), text, link) {}

Label::Label(const TRect& r, TStringView text, TView* link) : TLabel(r, text, link) {}

void Label::setTitle(const std::string& title) {
    delete[] const_cast<char*>(text);  // NOLINT
    text = newStr(title);
}

TColorAttr Label::mapColor(uchar index) noexcept {
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

}  // namespace shared
