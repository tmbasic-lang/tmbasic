#include "InputLine.h"
#include "shared/util/minmax.h"

namespace tmbasic {

InputLine::InputLine(const std::string& text, int aMaxLen)
    : TInputLine(TRect(0, 0, 10, 1), util::max(text.size() + 1, static_cast<size_t>(aMaxLen))) {
    memcpy(data, text.c_str(), text.size());
    data[text.size()] = 0;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

}  // namespace tmbasic
