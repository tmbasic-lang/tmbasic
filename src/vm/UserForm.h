#pragma once

#include "../common.h"
#include "UserControl.h"

namespace vm {

class UserForm {
   public:
    std::string name;
    std::string title;
    bool showCloseButton = true;
    bool showMaximizeButton = true;
    bool allowResize = false;
    std::vector<std::unique_ptr<UserControl>> controls;
    int minWidth = 0;
    int minHeight = 0;
    int initialWidth = 30;
    int initialHeight = 10;
    int maxWidth = 0;
    int maxHeight = 0;
};

}  // namespace vm
