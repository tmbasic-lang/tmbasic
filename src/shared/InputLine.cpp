#include "../shared/InputLine.h"

namespace shared {

InputLine::InputLine(const std::string& text, int width, int aMaxLen)
    : TInputLine(
          TRect(0, 0, width + 1 /*why is +1 needed?*/, 1),
          std::max(text.size() + 1, static_cast<size_t>(aMaxLen) + 1)) {
    memcpy(data, text.c_str(), text.size());
    data[text.size()] = 0;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

InputLine::InputLine(int number, int width, int aMaxLen) : InputLine(std::to_string(number), width, aMaxLen) {}

void InputLine::setText(const std::string& text) {
    assert(text.size() <= static_cast<size_t>(maxLen));
    snprintf(data, maxLen + 1, "%s", text.c_str());
}

}  // namespace shared
