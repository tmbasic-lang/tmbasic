#include "App.h"
#include "ProgramWindow.h"

#define Uses_MsgBox
#define Uses_TDeskTop
#define Uses_TKeys
#define Uses_TMenuItem
#define Uses_TStatusDef
#define Uses_TStatusItem
#define Uses_TSubMenu
#include <tvision/tv.h>

using namespace ui;

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
        *new TMenuItem("~R~edo", cmUndo, kbCtrlZ, hcNoContext, "Ctrl+Y") + newLine() +
        *new TMenuItem("Cu~t~", cmCut, kbCtrlX, hcNoContext, "Ctrl+X") +
        *new TMenuItem("~C~opy", cmCopy, kbCtrlC, hcNoContext, "Ctrl+C") +
        *new TMenuItem("~P~aste", cmPaste, kbCtrlV, hcNoContext, "Ctrl+V");

    auto& viewMenu = *new TSubMenu("~V~iew", kbAltE) +
        *new TMenuItem("~P~rogram", kCmdProgramContentsWindow, kbCtrlP, hcNoContext, "Ctrl+P") +
        *new TMenuItem("~I~mmediate", kCmdProgramContentsWindow, kbCtrlI, hcNoContext, "Ctrl+I");

    auto& programMenu = *new TSubMenu("~P~rogram", kbAltP) +
        *new TMenuItem("Add ~s~ubroutine...", kCmdProgramAddSubroutine, kbNoKey) +
        *new TMenuItem("Add ~f~unction...", kCmdProgramAddFunction, kbNoKey);

    auto& windowMenu = *new TSubMenu("~W~indow", kbAltW) +
        *new TMenuItem("~S~ize/move", cmResize, kbCtrlF5, hcNoContext, "Ctrl+F5") +
        *new TMenuItem("~Z~oom", cmZoom, kbNoKey) + *new TMenuItem("~N~ext", cmNext, kbF6, hcNoContext, "F6") +
        *new TMenuItem("~P~revious", cmPrev, kbShiftF6, hcNoContext, "Shift+F6") +
        *new TMenuItem("~C~lose", cmClose, kbCtrlW, hcNoContext, "Ctrl+W");

    auto& helpMenu = *new TSubMenu("~H~elp", kbAltH) + *new TMenuItem("~A~bout TMBASIC", kCmdHelpAbout, kbNoKey);

    r.b.y = r.a.y + 1;
    return new TMenuBar(r, fileMenu + editMenu + viewMenu + programMenu + windowMenu + helpMenu);
}

TStatusLine* App::initStatusLine(TRect r) {
    r.a.y = r.b.y - 1;
    return new TStatusLine(r, *new TStatusDef(0, 0xFFFF));
}

bool App::handleCommand(TEvent& event) {
    switch (event.message.command) {
        case cmNew:
            onFileNew();
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
    auto window = new ProgramWindow(getNewWindowRect(40, 15));
    deskTop->insert(window);
}
