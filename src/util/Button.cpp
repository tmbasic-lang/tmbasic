#include "Button.h"

namespace util {

static const auto kButtonMinWidth = 13;
static const auto kButtonHeight = 2;

static int getButtonWidth(TStringView title) {
    // we want the same amount of padding on both sides of the text
    auto len = cstrlen(title);
    auto width = len + 7;
    if (width < kButtonMinWidth) {
        if ((len % 2) != 0U) {
            width = kButtonMinWidth - 1;
        } else {
            width = kButtonMinWidth;
        }
    }
    return width;
}

Button::Button(const TRect& bounds, TStringView aTitle, ushort aCommand, ushort aFlags)
    : TButton(bounds, aTitle, aCommand, aFlags) {}

Button::Button(TStringView aTitle, ushort aCommand, ushort aFlags)
    : TButton(TRect(0, 0, getButtonWidth(aTitle), kButtonHeight), aTitle, aCommand, aFlags) {}

TColorAttr Button::mapColor(uchar index) noexcept {
    switch (index) {
        case 1:
            if (colorTextNormal.has_value()) {
                return *colorTextNormal;
            }
            break;
        case 2:
            if (colorTextDefault.has_value()) {
                return *colorTextDefault;
            }
            break;
        case 3:
            if (colorTextSelected.has_value()) {
                return *colorTextSelected;
            }
            break;
        case 4:
            if (colorTextDisabled.has_value()) {
                return *colorTextDisabled;
            }
            break;
        case 5:
            if (colorShortcutNormal.has_value()) {
                return *colorShortcutNormal;
            }
            break;
        case 6:
            if (colorShortcutDefault.has_value()) {
                return *colorShortcutDefault;
            }
            break;
        case 7:
            if (colorShortcutSelected.has_value()) {
                return *colorShortcutSelected;
            }
            break;
        case 8:
            if (colorShadow.has_value()) {
                return *colorShadow;
            }
            break;
        default:
            break;
    }

    return TButton::mapColor(index);
}

void Button::useDefaultColorScheme() {
    colorTextNormal.reset();
    colorTextDefault.reset();
    colorTextSelected.reset();
    colorTextDisabled.reset();
    colorShortcutNormal.reset();
    colorShortcutDefault.reset();
    colorShortcutSelected.reset();
}

void Button::useWhiteColorScheme() {
    colorTextNormal = { 0xF0 };
    colorTextDefault = { 0xF0 };
    colorTextSelected = { 0xF2 };
    colorTextDisabled = { 0xF0 };
    colorShortcutNormal = { 0xF0 };
    colorShortcutDefault = { 0xF0 };
    colorShortcutSelected = { 0xF2 };
}

void Button::setTitle(const std::string& text) {
    delete[] const_cast<char*>(title);  // NOLINT
    title = newStr(text);
}

}  // namespace util
