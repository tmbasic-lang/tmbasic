#include "InputLine.h"
#include "util/minmax.h"

namespace tmbasic {

InputLine::InputLine(const std::string& text, int width, int aMaxLen)
    : TInputLine(
          TRect(0, 0, width + 1 /*why is +1 needed?*/, 1),
          util::max(text.size() + 1, static_cast<size_t>(aMaxLen) + 1)) {
    memcpy(data, text.c_str(), text.size());
    data[text.size()] = 0;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

static std::string intToString(int number) {
    std::ostringstream s;
    s << number;
    return s.str();
}

InputLine::InputLine(int number, int width, int aMaxLen) : InputLine(intToString(number), width, aMaxLen) {}

}  // namespace tmbasic
