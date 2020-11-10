#pragma once

#include "common.h"
#include "constants.h"

namespace tmbasic {

class App : public TApplication {
   public:
    App(int argc, char** argv);
    void handleEvent(TEvent& event) override;

   private:
    static TMenuBar* initMenuBar(TRect r);
    static TStatusLine* initStatusLine(TRect r);
    bool handleCommand(TEvent& event);
    TRect getNewWindowRect(int width, int height);
    void onFileNew();
    void onProgramAddProcedure(bool function);
    void onHelpDocumentation();
    void onHelpBasicReference();
    TRect centeredRect(int width, int height);
    void openHelpTopic(ushort topic);

    int _newWindowX;
    int _newWindowY;
};

}  // namespace tmbasic
