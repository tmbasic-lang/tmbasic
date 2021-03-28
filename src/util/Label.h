#pragma once

#include "../common.h"

namespace util {

class Label : public TLabel {
   public:
    std::optional<TColorAttr> colorActive;
    std::optional<TColorAttr> colorInactive;
    std::optional<TColorAttr> colorFocused;
    std::optional<TColorAttr> colorSelected;
    std::optional<TColorAttr> colorDivider;
    explicit Label(const TRect& r);
    explicit Label(TStringView text, TView* link = nullptr);
    void setTitle(const std::string& title);
    TColorAttr mapColor(uchar index) override;
};

}  // namespace util
