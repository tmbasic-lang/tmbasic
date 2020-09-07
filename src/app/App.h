#pragma once
#include "tvision.h"

class App : public TApplication {
   public:
    App(int argc, char** argv);
    virtual void handleEvent(TEvent& event);

   private:
    static TMenuBar* initMenuBar(TRect r);
    static TStatusLine* initStatusLine(TRect r);
};
