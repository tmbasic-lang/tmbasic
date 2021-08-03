#include "ScrollBar.h"

namespace util {

ScrollBar::ScrollBar(const TRect& bounds) : TScrollBar(bounds) {}

TColorAttr ScrollBar::mapColor(uchar index) noexcept {
    switch (index) {
        case 1:
            if (colorPage.has_value()) {
                return *colorPage;
            }
            break;
        case 2:
            if (colorArrows.has_value()) {
                return *colorArrows;
            }
            break;
        case 3:
            if (colorIndicator.has_value()) {
                return *colorIndicator;
            }
            break;
        default:
            break;
    }
    return TScrollBar::mapColor(index);
}

void ScrollBar::useBlueColorScheme() {
    colorPage = TColorAttr(0x13);
    colorArrows = TColorAttr(0x13);
    colorIndicator = TColorAttr(0x13);
}

void ScrollBar::useCyanColorScheme() {
    colorPage = TColorAttr(0x31);
    colorArrows = TColorAttr(0x31);
    colorIndicator = TColorAttr(0x31);
}

void ScrollBar::useWhiteColorScheme() {
    colorPage = TColorAttr(0xF0);
    colorArrows = TColorAttr(0xF0);
    colorIndicator = TColorAttr(0xF0);
}

}  // namespace util
