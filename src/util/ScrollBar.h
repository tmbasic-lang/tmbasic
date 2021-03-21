#pragma once

#include "../common.h"

namespace util {

class ScrollBar : public TScrollBar {
   public:
    std::optional<TColorAttr> colorPage;
    std::optional<TColorAttr> colorArrows;
    std::optional<TColorAttr> colorIndicator;
    ScrollBar(const TRect& bounds);
    TColorAttr mapColor(uchar index) override;
    void useCyanColorScheme();
    void useWhiteColorScheme();
};

}  // namespace util
