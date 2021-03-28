#pragma once

#include "../common.h"

namespace util {

class Button : public TButton {
   public:
    std::optional<TColorAttr> colorTextNormal;
    std::optional<TColorAttr> colorTextDefault;
    std::optional<TColorAttr> colorTextSelected;
    std::optional<TColorAttr> colorTextDisabled;
    std::optional<TColorAttr> colorShortcutNormal;
    std::optional<TColorAttr> colorShortcutDefault;
    std::optional<TColorAttr> colorShortcutSelected;
    std::optional<TColorAttr> colorShadow;
    Button(const TRect& bounds, TStringView aTitle, ushort aCommand, ushort aFlags);
    Button(TStringView aTitle, ushort aCommand, ushort aFlags);
    TColorAttr mapColor(uchar index) override;
    void useDefaultColorScheme();
    void useWhiteColorScheme();
    void setTitle(const std::string& text);
    static int getButtonWidth(TStringView title);
};

}  // namespace util
