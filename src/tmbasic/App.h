#pragma once

#include "../common.h"
#include "tmbasic/ProgramWindow.h"

namespace tmbasic {

enum class TextEditorType { kFunction, kSubroutine, kGlobalVariable, kConstant, kType };

class App : public TApplication {
   public:
    App(int argc, char** argv);
    void idle() override;
    void handleEvent(TEvent& event) override;
    TPalette& getPalette() const override;

   private:
    void disableDefaultCommands();
    static TMenuBar* initMenuBar(TRect r);
    static TStatusLine* initStatusLine(TRect r);
    bool handleCommand(TEvent* event);
    TRect getNewWindowRect(int width, int height);
    static ProgramWindow* findProgramWindow(TDeskTop* deskTop);
    static bool closeProgramWindow(ProgramWindow* programWindow);
    void onFileNew();
    void onFileOpen();
    void onViewProgram();
    void onProgramAdd(TextEditorType type);
    void onHelpDocumentation();
    void onHelpBasicReference();
    TRect centeredRect(int width, int height);
    void openHelpTopic(uint16_t topic);
    void showNewProgramWindow(std::optional<std::string> filePath);
    void showNewEditorWindow(compiler::SourceMember* member);

    int _newWindowX;
    int _newWindowY;
    std::chrono::steady_clock::time_point _lastTimerTick;
};

}  // namespace tmbasic
