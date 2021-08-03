#include "ListViewer.h"

namespace util {

ListViewer::ListViewer(const TRect& bounds, ushort aNumCols, TScrollBar* aHScrollBar, TScrollBar* aVScrollBar)
    : TListViewer(bounds, aNumCols, aHScrollBar, aVScrollBar) {}

TColorAttr ListViewer::mapColor(uchar index) noexcept {
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
    colorActive = { 0x8F };
    colorInactive = { 0x87 };
    colorSelected = { 0x8E };
}

void ListViewer::useCyanPalette() {
    colorActive = { 0x3F };
    colorInactive = { 0x37 };
    colorSelected = { 0x3E };
}

}  // namespace util
