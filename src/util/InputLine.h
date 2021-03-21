#pragma once

#include "../common.h"

namespace util {

class InputLine : public TInputLine {
   public:
    explicit InputLine(const std::string& text = "", int width = 10, int aMaxLen = 1000);
    explicit InputLine(int number, int width = 10, int aMaxLen = 1000);
};

}  // namespace util
