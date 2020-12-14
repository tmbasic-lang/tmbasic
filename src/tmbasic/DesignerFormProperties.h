#pragma once

#include "../common.h"

namespace tmbasic {

class DesignerFormProperties {
   public:
    std::string name;
    std::string title;
    bool showCloseButton = true;
    bool showMaximizeButton = true;
    bool allowResize = false;
};

}  // namespace tmbasic
