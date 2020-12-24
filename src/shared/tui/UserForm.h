#pragma once

#include "../../common.h"
#include "UserControl.h"

namespace tui {

class UserForm {
   public:
    std::string name;
    std::string title;
    bool showCloseButton = true;
    bool showMaximizeButton = true;
    bool allowResize = false;
    std::vector<std::unique_ptr<UserControl>> controls;
};

}  // namespace tui
