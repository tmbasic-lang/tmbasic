#pragma once

#include "../common.h"

namespace tmbasic {

class CheckBoxes : public TCheckBoxes {
   public:
    CheckBoxes(std::initializer_list<std::string> labels);
    CheckBoxes(std::initializer_list<std::string> labels, std::initializer_list<bool> checks);
};

}  // namespace tmbasic
