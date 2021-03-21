#include "ListViewer.h"

namespace util {

ListViewer::ListViewer(const TRect& bounds, ushort aNumCols, TScrollBar* aHScrollBar, TScrollBar* aVScrollBar)
    : TListViewer(bounds, aNumCols, aHScrollBar, aVScrollBar) {}

TColorAttr ListViewer::mapColor(uchar index) {
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
    return TListViewer::mapColor(index);
}

void ListViewer::useDarkGrayPalette() {
    colorActive = TColorAttr(TColorDesired(TColorBIOS(15)), TColorDesired(TColorBIOS(8)));
    colorInactive = TColorAttr(TColorDesired(TColorBIOS(7)), TColorDesired(TColorBIOS(8)));
    colorSelected = TColorAttr(TColorDesired(TColorBIOS(14)), TColorDesired(TColorBIOS(8)));
}

void ListViewer::useCyanPalette() {
    colorActive = TColorAttr(TColorDesired(TColorBIOS(15)), TColorDesired(TColorBIOS(3)));
    colorInactive = TColorAttr(TColorDesired(TColorBIOS(7)), TColorDesired(TColorBIOS(3)));
    colorSelected = TColorAttr(TColorDesired(TColorBIOS(14)), TColorDesired(TColorBIOS(3)));
}

}  // namespace util
