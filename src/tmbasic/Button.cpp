#include "Button.h"

namespace tmbasic {

static const auto kButtonMinWidth = 13;
static const auto kButtonHeight = 2;

static int getButtonWidth(TStringView title) {
    // we want the same amount of padding on both sides of the text
    auto len = title.size();
    auto width = len + 7;
    if (width < kButtonMinWidth) {
        if (len % 2) {
            width = kButtonMinWidth - 1;
        } else {
            width = kButtonMinWidth;
        }
    }
    return width;
}

Button::Button(TStringView aTitle, ushort aCommand, ushort aFlags)
    : TButton(TRect(0, 0, getButtonWidth(aTitle), kButtonHeight), aTitle, aCommand, aFlags) {}

}  // namespace tmbasic
