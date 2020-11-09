#pragma once

#include "common.h"

namespace tmbasic {

class ProcedureWindow : public TWindow {
   public:
    const unsigned int kBufferSize = 100000;
    ProcedureWindow(const TRect&, bool function);
};

}  // namespace tmbasic
