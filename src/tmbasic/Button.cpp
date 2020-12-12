#include "Button.h"

namespace tmbasic {

static const auto kButtonWidth = 13;
static const auto kButtonHeight = 2;

Button::Button(TStringView aTitle, ushort aCommand, ushort aFlags)
    : TButton(TRect(0, 0, kButtonWidth, kButtonHeight), aTitle, aCommand, aFlags) {}

}  // namespace tmbasic
