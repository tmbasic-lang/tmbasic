#pragma once

#include "common.h"

namespace tmbasic {

class EditorWindow : public TWindow {
   public:
    const unsigned int kBufferSize = 100000;
    EditorWindow(const TRect&, bool function);
    ushort getHelpCtx() override;
};

}  // namespace tmbasic
