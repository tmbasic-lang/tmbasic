#pragma once

#include "../common.h"

namespace tmbasic {

class AboutDialog : public TDialog {
   public:
    AboutDialog();
    uint16_t getHelpCtx() override;
};

}  // namespace tmbasic
