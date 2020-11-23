#include "tmbasic/App.h"
#include "../../obj/helpfile.h"
#include "shared/util/membuf.h"
#include "tmbasic/EditorWindow.h"
#include "tmbasic/HelpResource.h"
#include "tmbasic/ProgramWindow.h"
#include "tmbasic/SourceProgram.h"
#include "tmbasic/events.h"

using util::membuf;

namespace tmbasic {

App::App(int argc, char** argv)
    : TProgInit(initStatusLine, initMenuBar, TApplication::initDeskTop), _newWindowX(2), _newWindowY(1) {
    disableDefaultCommands();
}

void App::idle() {
    TApplication::idle();

    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> msec = now - _lastTimerTick;
    if (msec.count() >= 100) {
        _lastTimerTick = now;
        message(deskTop, evBroadcast, kCmdTimerTick, nullptr);
    }
}

void App::handleEvent(TEvent& event) {
    if (event.what == evCommand && handleCommand(&event)) {
        clearEvent(event);
    }
    TApplication::handleEvent(event);
}

void App::disableDefaultCommands() {
    TCommandSet ts;
    ts.enableCmd(cmSave);
    ts.enableCmd(cmSaveAs);
    disableCommands(ts);
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
        *new TMenuItem("Add ~s~ubroutine", kCmdProgramAddSubroutine, kbNoKey) +
        *new TMenuItem("Add ~f~unction", kCmdProgramAddFunction, kbNoKey) +
        *new TMenuItem("Add ~g~lobal variable", kCmdProgramAddGlobalVariable, kbNoKey) +
        *new TMenuItem("Add ~c~onstant", kCmdProgramAddConstant, kbNoKey) +
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

TStatusLine* App::initStatusLine(TRect r) {
    r.a.y = r.b.y - 1;

    auto& appStatusDef = *new TStatusDef(0, 0xFFFF) + *new TStatusItem("~Ctrl+N~ New program", kbNoKey, cmNew) +
        *new TStatusItem("~Ctrl+O~ Open program", kbNoKey, cmOpen) + *new TStatusItem("~Ctrl+Q~ Exit", kbNoKey, cmQuit);

    auto& editorWindowStatusDef = *new TStatusDef(hcide_editorWindow, hcide_editorWindow) +
        *new TStatusItem("~Ctrl+W~ Close editor", kbNoKey, cmClose);
    editorWindowStatusDef.next = &appStatusDef;

    auto& programWindowStatusDef = *new TStatusDef(hcide_programWindow, hcide_programWindow) +
        *new TStatusItem("~Alt+1~ Add sub", kbAlt1, kCmdProgramAddSubroutine) +
        *new TStatusItem("~Alt+2~ Add function", kbAlt2, kCmdProgramAddFunction) +
        *new TStatusItem("~Alt+3~ Add global", kbAlt3, kCmdProgramAddGlobalVariable) +
        *new TStatusItem("~Alt+4~ Add constant", kbAlt4, kCmdProgramAddConstant) +
        *new TStatusItem("~Alt+5~ Add type", kbAlt5, kCmdProgramAddType);
    programWindowStatusDef.next = &editorWindowStatusDef;

    return new TStatusLine(r, programWindowStatusDef);
}

bool App::handleCommand(TEvent* event) {
    switch (event->message.command) {
        case cmNew:
            onFileNew();
            return true;

        case cmOpen:
            onFileOpen();
            return true;

        case cmSave:
            message(deskTop, evBroadcast, kCmdProgramSave, nullptr);
            return true;

        case cmSaveAs:
            message(deskTop, evBroadcast, kCmdProgramSaveAs, nullptr);
            return true;

        case kCmdProgramAddSubroutine:
            onProgramAdd(TextEditorType::kSubroutine);
            return true;

        case kCmdProgramAddFunction:
            onProgramAdd(TextEditorType::kFunction);
            return true;

        case kCmdProgramAddGlobalVariable:
            onProgramAdd(TextEditorType::kGlobalVariable);
            return true;

        case kCmdProgramAddConstant:
            onProgramAdd(TextEditorType::kConstant);
            return true;

        case kCmdProgramAddType:
            onProgramAdd(TextEditorType::kType);
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

ProgramWindow* App::findProgramWindow(TDeskTop* deskTop) {
    ProgramWindow* programWindow = nullptr;
    message(deskTop, evBroadcast, kCmdFindProgramWindow, &programWindow);
    return programWindow;
}

bool App::closeProgramWindow(ProgramWindow* programWindow) {
    if (programWindow) {
        programWindow->close();
        if (programWindow->frame) {
            // the program window is still open, so the user must have clicked cancel
            return false;
        }
    }
    return true;
}

void App::showNewProgramWindow(std::optional<std::string> filePath) {
    auto sourceProgram = std::make_unique<SourceProgram>();
    if (filePath.has_value()) {
        try {
            sourceProgram->load(*filePath);
        } catch (std::runtime_error& ex) {
            messageBox(std::string("There was an error opening the file: ") + ex.what(), mfError | mfOKButton);
            return;
        } catch (...) {
            messageBox(std::string("There was an error opening the file."), mfError | mfOKButton);
            return;
        }
    }

    auto* window = new ProgramWindow(
        TRect(0, 0, 60, 15), std::move(sourceProgram), filePath,
        [this](auto* member) -> void { showNewEditorWindow(member); });
    deskTop->insert(window);
}

void App::showNewEditorWindow(SourceMember* member) {
    // is there already an editor open for this member?
    FindEditorWindowEventArgs e = { 0 };
    e.member = member;
    message(deskTop, evBroadcast, kCmdFindEditorWindow, &e);
    if (e.window) {
        e.window->focus();
    } else {
        auto window = new EditorWindow(getNewWindowRect(82, 30), member, [this]() -> void {
            // onUpdated
            auto* programWindow = findProgramWindow(deskTop);
            if (programWindow) {
                programWindow->redrawListItems();
            }
        });
        deskTop->insert(window);
    }
}

void App::onFileNew() {
    // close the existing program if there is one
    auto* programWindow = findProgramWindow(deskTop);
    if (!closeProgramWindow(programWindow)) {
        return;
    }

    showNewProgramWindow(std::optional<std::string>());
}

void App::onFileOpen() {
    auto* programWindow = findProgramWindow(deskTop);

    auto* d = new TFileDialog("*.bas", "Open Program (.BAS)", "~N~ame", fdOpenButton, 100);
    if (deskTop->execView(d) != cmCancel) {
        if (!closeProgramWindow(programWindow)) {
            destroy(d);
            return;
        }

        char fileName[MAXPATH];
        d->getFileName(fileName);
        showNewProgramWindow(fileName);
    }
    destroy(d);
}

void App::onProgramAdd(TextEditorType type) {
    auto* programWindow = findProgramWindow(deskTop);
    if (!programWindow) {
        return;
    }

    auto memberType = SourceMemberType::kConstant;
    auto source = std::string();
    auto selectionStart = 0;
    auto selectionEnd = 0;

    switch (type) {
        case TextEditorType::kConstant:
            memberType = SourceMemberType::kConstant;
            source = "const untitled = 0\n";
            selectionStart = 6;
            selectionEnd = 14;
            break;
        case TextEditorType::kGlobalVariable:
            memberType = SourceMemberType::kGlobalVariable;
            source = "dim untitled as number\n";
            selectionStart = 4;
            selectionEnd = 12;
            break;
        case TextEditorType::kFunction:
            memberType = SourceMemberType::kProcedure;
            source = "function untitled() as integer\n\nend function\n";
            selectionStart = 9;
            selectionEnd = 17;
            break;
        case TextEditorType::kSubroutine:
            memberType = SourceMemberType::kProcedure;
            source = "sub untitled()\n\nend sub\n";
            selectionStart = 4;
            selectionEnd = 12;
            break;
        case TextEditorType::kType:
            memberType = SourceMemberType::kType;
            source = "type Untitled\n\nend type\n";
            selectionStart = 5;
            selectionEnd = 13;
            break;
        default:
            assert(false);
            break;
    }

    auto sourceMember = std::make_unique<SourceMember>(memberType, source, selectionStart, selectionEnd);
    auto* sourceMemberPtr = sourceMember.get();
    programWindow->addNewSourceMember(std::move(sourceMember));

    showNewEditorWindow(sourceMemberPtr);
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

void App::openHelpTopic(uint16_t topic) {
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

}  // namespace tmbasic

int main(int argc, char** argv) {
    tmbasic::App app(argc, argv);
    app.run();
    app.shutDown();
    return 0;
}
