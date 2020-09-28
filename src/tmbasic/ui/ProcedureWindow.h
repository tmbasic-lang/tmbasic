#pragma once

#include "common.h"

namespace ui {

class ProcedureWindow : public TWindow {
   public:
    const unsigned int kBufferSize = 100000;
    ProcedureWindow(const TRect&, bool function);
};

}  // namespace ui
