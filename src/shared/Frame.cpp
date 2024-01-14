#include "Frame.h"

namespace shared {

Frame::Frame(const TRect& bounds) : TFrame(bounds) {}

TColorAttr Frame::mapColor(uchar index) noexcept {
    switch (index) {
        case 1:
            if (colorPassiveFrame.has_value()) {
                return *colorPassiveFrame;
            }
            break;
        case 2:
            if (colorPassiveTitle.has_value()) {
                return *colorPassiveTitle;
            }
            break;
        case 3:
            if (colorActiveFrame.has_value()) {
                return *colorActiveFrame;
            }
            break;
        case 4:
            if (colorActiveTitle.has_value()) {
                return *colorActiveTitle;
            }
            break;
        case 5:
            if (colorIcons.has_value()) {
                return *colorIcons;
            }
            break;
        default:
            break;
    }
    return TFrame::mapColor(index);
}

}  // namespace shared
