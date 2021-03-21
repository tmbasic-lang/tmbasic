#pragma once

#include "../common.h"
#include "tmbasic/ProgramWindow.h"

namespace tmbasic {

enum class EditorType { kFunction, kSubroutine, kGlobalVariable, kConstant, kType };

enum class DesignerType { kForm, kCustomControl };

class App : public TApplication {
   public:
    static std::array<char, 9> helpWindowPalette;

    App(int argc, char** argv);
    void idle() override;
    void handleEvent(TEvent& event) override;
    TPalette& getPalette() const override;

   private:
    static TMenuBar* initMenuBar(TRect r);
    static TStatusLine* initStatusLine(TRect r);
    bool handleCommand(TEvent* event);
    TRect getNewWindowRect(int width, int height);
    static ProgramWindow* findProgramWindow(TDeskTop* deskTop);
    static bool closeProgramWindow(ProgramWindow* programWindow);
    void onFileNew();
    void onFileOpen();
    static void onViewProgram();
    void onProgramAddTextEditor(EditorType type);
    void onProgramAddDesigner(DesignerType type);
    void onProgramAddPicture();
    void onHelpDocumentation();
    static void onHelpAbout();
    static TRect centeredRect(int width, int height);
    void openHelpTopic(uint16_t topic);
    void showNewProgramWindow(std::optional<std::string> filePath);
    void openEditorOrDesignerWindow(compiler::SourceMember* member);
    void showEditorWindow(compiler::SourceMember* member);
    void showDesignerWindow(compiler::SourceMember* member);
    void showPictureWindow(compiler::SourceMember* member);

    int _newWindowX;
    int _newWindowY;
    std::chrono::steady_clock::time_point _lastTimerTick;
};

}  // namespace tmbasic
