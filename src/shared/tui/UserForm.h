#pragma once

#include "../../common.h"

namespace tui {

class UserForm {
   public:
    std::string name;
    std::string title;
    bool showCloseButton = true;
    bool showMaximizeButton = true;
    bool allowResize = false;
};

}  // namespace tui
