#pragma once

#include "../common.h"

namespace shared {

class CheckBoxes : public TCheckBoxes {
   public:
    explicit CheckBoxes(const std::vector<std::string>& labels);
    CheckBoxes(const std::vector<std::string>& labels, const std::vector<bool>& checks);
};

}  // namespace shared
