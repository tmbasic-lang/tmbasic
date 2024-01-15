#pragma once

#include "../common.h"
#include "vm/BasicConsoleView.h"
#include "shared/ViewPtr.h"

namespace vm {

class BasicApp : public TApplication {
   public:
    static BasicApp* createInstance();
    static std::unique_ptr<BasicApp> instance;

    shared::ViewPtr<BasicConsoleView> console;

    BasicApp();
    virtual ~BasicApp();
    void forceScreenUpdate();

   private:
    static TStatusLine* initBasicStatusLine(TRect r);
    static TMenuBar* initBasicMenuBar(TRect r);
    static TDeskTop* initBasicDeskTop(TRect r);
};

}  // namespace vm
