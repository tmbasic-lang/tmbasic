#include "common.h"
#include "App.h"
#include "HelpResource.h"
#include "EditorWindow.h"
#include "ProgramWindow.h"
#include "constants.h"
#include "helpfile.h"

namespace tmbasic {

struct membuf : std::streambuf {
    membuf(char* begin, char* end) { this->setg(begin, begin, end); }

    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in)
        override {
        if (dir == std::ios_base::cur) {
            gbump(off);
        } else if (dir == std::ios_base::end) {
            setg(eback(), egptr() + off, egptr());
        } else if (dir == std::ios_base::beg) {
            setg(eback(), eback() + off, egptr());
        }
        return gptr() - eback();
    }

    pos_type seekpos(pos_type sp, std::ios_base::openmode which) override {
        return seekoff(sp - pos_type(off_type(0)), std::ios_base::beg, which);
    }
};

class App : public TApplication {
   public:
    App(int argc, char** argv)
        : TProgInit(initStatusLine, initMenuBar, TApplication::initDeskTop), _newWindowX(0), _newWindowY(0) {
        disableDefaultCommands();
    }
    void handleEvent(TEvent& event) override {
        if (event.what == evCommand && handleCommand(event)) {
            clearEvent(event);
        }
        TApplication::handleEvent(event);
    }

   private:
    void disableDefaultCommands() {
        TCommandSet ts;
        ts.enableCmd(cmSave);
        ts.enableCmd(cmSaveAs);
        disableCommands(ts);
    }
    static TMenuBar* initMenuBar(TRect r) {
        auto& fileMenu = *new TSubMenu("~F~ile", kbAltF) +
            *new TMenuItem("~N~ew", cmNew, kbCtrlN, hcNoContext, "Ctrl+N") +
            *new TMenuItem("~O~pen...", cmOpen, kbCtrlO, hcNoContext, "Ctrl+O") +
            *new TMenuItem("~S~ave", cmSave, kbCtrlS, hcNoContext, "Ctrl+S") +
            *new TMenuItem("S~a~ve as...", cmSaveAs, kbCtrlS) + newLine() +
            *new TMenuItem("E~x~it", cmQuit, kbCtrlQ, hcNoContext, "Ctrl+Q");

        auto& editMenu = *new TSubMenu("~E~dit", kbAltE) +
            *new TMenuItem("~U~ndo", cmUndo, kbCtrlZ, hcNoContext, "Ctrl+Z") +
            *new TMenuItem("~R~edo", cmUndo, kbCtrlY, hcNoContext, "Ctrl+Y") + newLine() +
            *new TMenuItem("Cu~t~", cmCut, kbCtrlX, hcNoContext, "Ctrl+X") +
            *new TMenuItem("~C~opy", cmCopy, kbCtrlC, hcNoContext, "Ctrl+C") +
            *new TMenuItem("~P~aste", cmPaste, kbCtrlV, hcNoContext, "Ctrl+V");

        auto& viewMenu = *new TSubMenu("~V~iew", kbAltE) +
            *new TMenuItem("~P~rogram", kCmdProgramContentsWindow, kbCtrlP, hcNoContext, "Ctrl+P") +
            *new TMenuItem("~I~mmediate", kCmdProgramContentsWindow, kbCtrlI, hcNoContext, "Ctrl+I");

        auto& programMenu = *new TSubMenu("~P~rogram", kbAltP) +
            *new TMenuItem("~R~un", kCmdProgramRun, kbF5, hcNoContext, "F5") + newLine() +
            *new TMenuItem("Add ~s~ubroutine", kCmdProgramAddSubroutine, kbNoKey) +
            *new TMenuItem("Add ~f~unction", kCmdProgramAddFunction, kbNoKey) +
            *new TMenuItem("Add ~g~lobal variable", kCmdProgramAddGlobalVariable, kbNoKey) +
            *new TMenuItem("Add ~t~ype", kCmdProgramAddType, kbNoKey);

        auto& windowMenu = *new TSubMenu("~W~indow", kbAltW) +
            *new TMenuItem("~S~ize/move", cmResize, kbCtrlF5, hcNoContext, "Ctrl+F5") +
            *new TMenuItem("~Z~oom", cmZoom, kbNoKey) + *new TMenuItem("~N~ext", cmNext, kbF6, hcNoContext, "F6") +
            *new TMenuItem("~P~revious", cmPrev, kbShiftF6, hcNoContext, "Shift+F6") +
            *new TMenuItem("~C~lose", cmClose, kbCtrlW, hcNoContext, "Ctrl+W");

        auto& helpMenu = *new TSubMenu("~H~elp", kbAltH) +
            *new TMenuItem("~D~ocumentation", kCmdHelpDocumentation, kbNoKey) +
            *new TMenuItem("~B~ASIC reference", kCmdHelpBasicReference, kbNoKey) + newLine() +
            *new TMenuItem("~A~bout TMBASIC", kCmdHelpAbout, kbNoKey);

        r.b.y = r.a.y + 1;
        return new TMenuBar(r, fileMenu + editMenu + viewMenu + programMenu + windowMenu + helpMenu);
    }

    static TStatusLine* initStatusLine(TRect r) {
        r.a.y = r.b.y - 1;

        auto& appStatusDef = *new TStatusDef(0, 0xFFFF) + *new TStatusItem("~Ctrl+N~ New program", kbNoKey, cmNew) +
            *new TStatusItem("~Ctrl+O~ Open program", kbNoKey, cmOpen) +
            *new TStatusItem("~Ctrl+Q~ Exit", kbNoKey, cmQuit);

        auto& editorWindowStatusDef = *new TStatusDef(hcide_editorWindow, hcide_editorWindow) +
            *new TStatusItem("~Ctrl+W~ Close editor", kbNoKey, cmClose);
        editorWindowStatusDef.next = &appStatusDef;

        auto& programWindowStatusDef = *new TStatusDef(hcide_programWindow, hcide_programWindow) +
            *new TStatusItem("~Alt+1~ Add subroutine", kbAlt1, kCmdProgramAddSubroutine) +
            *new TStatusItem("~Alt+2~ Add function", kbAlt2, kCmdProgramAddFunction) +
            *new TStatusItem("~Alt+3~ Add global", kbAlt3, kCmdProgramAddGlobalVariable) +
            *new TStatusItem("~Alt+4~ Add type", kbAlt4, kCmdProgramAddGlobalVariable);
        programWindowStatusDef.next = &editorWindowStatusDef;

        return new TStatusLine(r, programWindowStatusDef);
    }

    bool handleCommand(TEvent& event) {
        switch (event.message.command) {
            case cmNew:
                onFileNew();
                return true;

            case cmOpen:
                onFileOpen();
                return true;

            case cmSave:
                onFileSave();
                return true;

            case kCmdProgramAddSubroutine:
                onProgramAddProcedure(false);
                return true;

            case kCmdProgramAddFunction:
                onProgramAddProcedure(true);
                return true;

            case kCmdHelpBasicReference:
                onHelpBasicReference();
                return true;

            case kCmdHelpDocumentation:
                onHelpDocumentation();
                return true;

            case kCmdHelpAbout:
                messageBox("TMBASIC\n(C) 2020 Brian Luft\ngithub.com/electroly/tmbasic", mfInformation | mfOKButton);
                return true;

            case cmQuit: {
                bool cancel = false;
                message(deskTop, evBroadcast, kCmdAppExit, &cancel);
                return cancel;
            }

            default:
                return false;
        }
    }

    TRect getNewWindowRect(int width, int height) {
        auto maxX = deskTop->size.x - width;
        auto x = _newWindowX;
        if (x > maxX) {
            x = maxX;
        }
        if (x < 0) {
            x = 0;
        }

        auto maxY = deskTop->size.y - height;
        auto y = _newWindowY;
        if (y > maxY) {
            y = maxY;
        }
        if (y < 0) {
            y = 0;
        }

        _newWindowX += 2;
        _newWindowY++;
        if (_newWindowX > deskTop->size.x / 2)
            _newWindowX = 0;
        if (_newWindowY > deskTop->size.y / 2)
            _newWindowY = 0;

        return TRect(x, y, x + width, y + height);
    }

    static ProgramWindow* findProgramWindow(TDeskTop* deskTop) {
        ProgramWindow* programWindow = nullptr;
        message(deskTop, evBroadcast, kCmdFindProgramWindow, &programWindow);
        return programWindow;
    }

    static bool closeProgramWindow(ProgramWindow* programWindow) {
        if (programWindow) {
            programWindow->close();
            if (programWindow->frame) {
                // the program window is still open, so the user must have clicked cancel
                return false;
            }
        }
        return true;
    }

    void onFileNew() {
        // close the existing program if there is one
        auto* programWindow = findProgramWindow(deskTop);
        if (!closeProgramWindow(programWindow)) {
            return;
        }

        auto* window = new ProgramWindow(getNewWindowRect(60, 15), std::optional<std::string>());
        deskTop->insert(window);
    }

    void onFileOpen() {
        auto* programWindow = findProgramWindow(deskTop);

        auto* d = new TFileDialog("*.bas", "Open Program (.BAS)", "~N~ame", fdOpenButton, 100);
        if (deskTop->execView(d) != cmCancel) {
            if (!closeProgramWindow(programWindow)) {
                destroy(d);
                return;
            }

            char fileName[MAXPATH];
            d->getFileName(fileName);
            deskTop->insert(new ProgramWindow(getNewWindowRect(60, 15), std::string(fileName)));
        }
        destroy(d);
    }

    void onFileSave() { message(deskTop, evBroadcast, kCmdProgramSave, nullptr); }

    void onProgramAddProcedure(bool function) {
        auto window = new EditorWindow(getNewWindowRect(70, 15), function);
        deskTop->insert(window);
    }

    void onHelpDocumentation() { openHelpTopic(hcdoc); }

    void onHelpBasicReference() { openHelpTopic(hcbasic); }

    TRect centeredRect(int width, int height) {
        auto x = (deskTop->size.x - width) / 2;
        auto y = (deskTop->size.y - height) / 2;
        return TRect(x, y, x + width, y + height);
    }

    void openHelpTopic(ushort topic) {
        HelpResource helpResource;
        auto buf = new membuf(reinterpret_cast<char*>(helpResource.start), reinterpret_cast<char*>(helpResource.end));
        auto stream = new iopstream(buf);
        auto helpFile = new THelpFile(*stream);
        auto helpWindow = new THelpWindow(helpFile, topic);
        deskTop->insert(helpWindow);
        auto width = 85;
        auto maxWidth = deskTop->size.x - 10;
        if (width > maxWidth) {
            width = maxWidth;
        } else if (width < 40) {
            width = 40;
        }
        auto height = deskTop->size.y * 0.8;
        if (height > 35) {
            height = 35;
        } else if (height < 10) {
            height = 10;
        }
        auto rect = getNewWindowRect(width, height);
        helpWindow->locate(rect);
    }

    int _newWindowX;
    int _newWindowY;
};

}  // namespace tmbasic

int main(int argc, char** argv) {
    tmbasic::App app(argc, argv);
    app.run();
    app.shutDown();
    return 0;
}
