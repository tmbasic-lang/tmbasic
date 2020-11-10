#include "common.h"
#include "App.h"
#include "HelpResource.h"
#include "ProcedureWindow.h"
#include "ProgramWindow.h"
#include "helpfile.h"

namespace tmbasic {

App::App(int argc, char** argv)
    : TProgInit(initStatusLine, initMenuBar, TApplication::initDeskTop), _newWindowX(0), _newWindowY(0) {}

void App::handleEvent(TEvent& event) {
    TApplication::handleEvent(event);
    if (event.what == evCommand && handleCommand(event)) {
        clearEvent(event);
    }
}

TMenuBar* App::initMenuBar(TRect r) {
    auto& fileMenu = *new TSubMenu("~F~ile", kbAltF) + *new TMenuItem("~N~ew", cmNew, kbCtrlN, hcNoContext, "Ctrl+N") +
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
        *new TMenuItem("Add ~s~ubroutine", kCmdProgramAddSubroutine, kbF2, hcNoContext, "F2") +
        *new TMenuItem("Add ~f~unction", kCmdProgramAddFunction, kbF3, hcNoContext, "F3") +
        *new TMenuItem("Add global ~v~ariable", kCmdProgramAddGlobalVariable, kbF4, hcNoContext, "F4");

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

TStatusLine* App::initStatusLine(TRect r) {
    r.a.y = r.b.y - 1;
    return new TStatusLine(
        r,
        *new TStatusDef(0, 0xFFFF) + *new TStatusItem("~F1~ Help", kbF1, cmHelp) +
            *new TStatusItem("~F2~ Add subroutine", kbF2, kCmdProgramAddSubroutine) +
            *new TStatusItem("~F3~ Add function", kbF3, kCmdProgramAddFunction) +
            *new TStatusItem("~F4~ Add global variable", kbF4, kCmdProgramAddGlobalVariable) +
            *new TStatusItem("~F5~ Run", kbF5, kCmdProgramRun));
}

bool App::handleCommand(TEvent& event) {
    switch (event.message.command) {
        case cmNew:
            onFileNew();
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
            messageBox(
                std::string("TMBASIC\n(C) 2020 Brian Luft\ngithub.com/electroly/tmbasic"), mfInformation | mfOKButton);
            return true;

        default:
            return false;
    }
}

TRect App::getNewWindowRect(int width, int height) {
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

void App::onFileNew() {
    auto window = newProgramWindow(getNewWindowRect(40, 15));
    deskTop->insert(window);
}

void App::onProgramAddProcedure(bool function) {
    auto window = new ProcedureWindow(getNewWindowRect(70, 15), function);
    deskTop->insert(window);
}

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

void App::openHelpTopic(ushort topic) {
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

void App::onHelpDocumentation() {
    openHelpTopic(hcdoc);
}

void App::onHelpBasicReference() {
    openHelpTopic(hcbasic);
}

TRect App::centeredRect(int width, int height) {
    auto x = (deskTop->size.x - width) / 2;
    auto y = (deskTop->size.y - height) / 2;
    return TRect(x, y, x + width, y + height);
}

}  // namespace tmbasic
