#pragma once

#include "common.h"

namespace tmbasic {

const ushort kCmdHelpBasicReference = 100;
const ushort kCmdHelpDocumentation = 101;
const ushort kCmdHelpAbout = 102;
const ushort kCmdProgramContentsWindow = 103;
const ushort kCmdProgramAddSubroutine = 104;
const ushort kCmdProgramAddFunction = 105;
const ushort kCmdProgramAddGlobalVariable = 106;
const ushort kCmdProgramRun = 107;

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
