#pragma once

#include "../common.h"
#include "vm/BasicConsoleView.h"
#include "util/ViewPtr.h"

namespace vm {

class BasicApp : public TApplication {
   public:
    static BasicApp* createInstance();
    static std::unique_ptr<BasicApp> instance;

    util::ViewPtr<BasicConsoleView> console;

    BasicApp();
    virtual ~BasicApp();

   private:
    TPoint _cursorPoint{ 0, 0 };
    TColorAttr _colorAttr{ 0x07 };

    static TStatusLine* initBasicStatusLine(TRect r);
    static TMenuBar* initBasicMenuBar(TRect r);
    static TDeskTop* initBasicDeskTop(TRect r);
};

}  // namespace vm
