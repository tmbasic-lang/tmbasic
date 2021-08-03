#pragma once

#include "../common.h"

namespace util {

class Frame : public TFrame {
   public:
    std::optional<TColorAttr> colorPassiveFrame;
    std::optional<TColorAttr> colorPassiveTitle;
    std::optional<TColorAttr> colorActiveFrame;
    std::optional<TColorAttr> colorActiveTitle;
    std::optional<TColorAttr> colorIcons;
    explicit Frame(const TRect& bounds);
    TColorAttr mapColor(uchar index) noexcept override;
};

}  // namespace util
