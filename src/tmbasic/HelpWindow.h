#pragma once

#include "../common.h"

namespace tmbasic {

class HelpWindow : public TWindow {
   public:
    HelpWindow(THelpFile* hFile, ushort context);
    TPalette& getPalette() const override;
};

}  // namespace tmbasic
