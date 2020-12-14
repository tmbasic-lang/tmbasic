#pragma once

#include "../common.h"

namespace tmbasic {

class InputLine : public TInputLine {
   public:
    explicit InputLine(const std::string& text = "", int aMaxLen = 1000);
};

}  // namespace tmbasic
