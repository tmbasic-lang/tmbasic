#pragma once

#include "common.h"

namespace ui {

const ushort kCmdHelpBasicReference = 100;
const ushort kCmdHelpAbout = 101;
const ushort kCmdProgramContentsWindow = 102;
const ushort kCmdProgramAddSubroutine = 103;
const ushort kCmdProgramAddFunction = 104;
const ushort kCmdProgramAddGlobalVariable = 105;
const ushort kCmdProgramRun = 106;

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
    void onHelpBasicReference();
    TRect centeredRect(int width, int height);

    int _newWindowX;
    int _newWindowY;
};

}  // namespace ui
