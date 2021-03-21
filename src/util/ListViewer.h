#pragma once

#include "../common.h"

namespace util {

class ListViewer : public TListViewer {
   public:
    std::optional<TColorAttr> colorActive;
    std::optional<TColorAttr> colorInactive;
    std::optional<TColorAttr> colorFocused;
    std::optional<TColorAttr> colorSelected;
    std::optional<TColorAttr> colorDivider;
    ListViewer(const TRect& bounds, ushort aNumCols, TScrollBar* aHScrollBar, TScrollBar* aVScrollBar);
    TColorAttr mapColor(uchar index) override;
    void useDarkGrayPalette();
    void useCyanPalette();
};

}  // namespace util
