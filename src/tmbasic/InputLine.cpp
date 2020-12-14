#include "InputLine.h"

namespace tmbasic {

InputLine::InputLine(const std::string& text, int aMaxLen)
    : TInputLine(TRect(0, 0, 10, 1), max(text.size() + 1, aMaxLen)) {
    snprintf(data, maxLen + 1, "%s", text.c_str());
}

}  // namespace tmbasic
