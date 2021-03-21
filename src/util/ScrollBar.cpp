#include "ScrollBar.h"

namespace util {

ScrollBar::ScrollBar(const TRect& bounds) : TScrollBar(bounds) {}

TColorAttr ScrollBar::mapColor(uchar index) {
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

void ScrollBar::useCyanColorScheme() {
    colorPage = TColorAttr(TColorDesired(TColorBIOS(1)), TColorDesired(TColorBIOS(3)));
    colorArrows = TColorAttr(TColorDesired(TColorBIOS(1)), TColorDesired(TColorBIOS(3)));
    colorIndicator = TColorAttr(TColorDesired(TColorBIOS(1)), TColorDesired(TColorBIOS(3)));
}

void ScrollBar::useWhiteColorScheme() {
    colorPage = TColorAttr(TColorDesired(TColorBIOS(0)), TColorDesired(TColorBIOS(15)));
    colorArrows = TColorAttr(TColorDesired(TColorBIOS(0)), TColorDesired(TColorBIOS(15)));
    colorIndicator = TColorAttr(TColorDesired(TColorBIOS(0)), TColorDesired(TColorBIOS(15)));
}

}  // namespace util
