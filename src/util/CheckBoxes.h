#pragma once

#include "../common.h"

namespace util {

class CheckBoxes : public TCheckBoxes {
   public:
    CheckBoxes(std::vector<std::string> labels);
    CheckBoxes(std::vector<std::string> labels, std::vector<bool> checks);
};

}  // namespace util
