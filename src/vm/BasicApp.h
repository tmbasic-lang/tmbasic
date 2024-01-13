#pragma once

#include "../common.h"
#include "vm/BasicBackground.h"

namespace vm {

class BasicApp : public TApplication {
   public:
    BasicBackground* background;

    BasicApp();
    void run() override;
    void suspend() override;
    void resume() override;
    bool isActive();

   private:
    bool _active = false;
    TPoint _cursorPoint{ 0, 0 };
    TColorAttr _colorAttr{ 0x07 };

    static TStatusLine* initBasicStatusLine(TRect r);
    static TMenuBar* initBasicMenuBar(TRect r);
    static TDeskTop* initBasicDeskTop(TRect r);
};

}  // namespace vm
