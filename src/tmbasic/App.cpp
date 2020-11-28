#include "tmbasic/App.h"
#include "../../obj/helpfile.h"
#include "compiler/SourceProgram.h"
#include "shared/util/membuf.h"
#include "tmbasic/DesignerWindow.h"
#include "tmbasic/EditorWindow.h"
#include "tmbasic/HelpResource.h"
#include "tmbasic/HelpWindow.h"
#include "tmbasic/ProgramWindow.h"
#include "tmbasic/constants.h"
#include "tmbasic/events.h"

using compiler::SourceMember;
using compiler::SourceMemberType;
using compiler::SourceProgram;
using util::membuf;

namespace tmbasic {

char App::helpWindowPalette[9] = {};

App::App(int argc, char** argv)
    : TProgInit(initStatusLine, initMenuBar, TApplication::initDeskTop), _newWindowX(2), _newWindowY(1) {
    disableDefaultCommands();
    onFileNew();
}

void App::idle() {
    TApplication::idle();

    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> msec = now - _lastTimerTick;
    if (msec.count() >= 250) {
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
        *new TMenuItem("~P~rogram", kCmdProgramContentsWindow, kbCtrlP, hcNoContext, "Ctrl+P");

    auto& programMenu = *new TSubMenu("~P~rogram", kbAltP) +
        *new TMenuItem("~R~un", kCmdProgramRun, kbF5, hcNoContext, "F5") + newLine() +
        *new TMenuItem("Add ~s~ubroutine", kCmdProgramAddSubroutine, kbNoKey) +
        *new TMenuItem("Add ~f~unction", kCmdProgramAddFunction, kbNoKey) +
        *new TMenuItem("Add ~g~lobal variable", kCmdProgramAddGlobalVariable, kbNoKey) +
        *new TMenuItem("Add ~c~onstant", kCmdProgramAddConstant, kbNoKey) +
        *new TMenuItem("Add ~t~ype", kCmdProgramAddType, kbNoKey) +
        *new TMenuItem("Add f~o~rm", kCmdProgramAddForm, kbNoKey) +
        *new TMenuItem("Add c~u~stom control", kCmdProgramAddCustomControl, kbNoKey);

    auto& formMenu = *new TSubMenu("~D~esign", kbAltD) + *new TMenuItem("Add ~b~utton", kCmdHelpAbout, kbNoKey) +
        *new TMenuItem("Add ~c~heck box", kCmdHelpAbout, kbNoKey) +
        *new TMenuItem("Add ~g~roup box", kCmdHelpAbout, kbNoKey) +
        *new TMenuItem("Add ~l~abel", kCmdHelpAbout, kbNoKey) +
        *new TMenuItem("Add list bo~x~", kCmdHelpAbout, kbNoKey) +
        *new TMenuItem("Add ~r~adio button", kCmdHelpAbout, kbNoKey) +
        *new TMenuItem("Add ~s~croll bar", kCmdHelpAbout, kbNoKey) +
        *new TMenuItem("Add ~t~ext box", kCmdHelpAbout, kbNoKey) + newLine() +
        *new TMenuItem("Add c~u~stom control", kCmdHelpAbout, kbNoKey);

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
    return new TMenuBar(r, fileMenu + editMenu + viewMenu + programMenu + formMenu + windowMenu + helpMenu);
}

TStatusLine* App::initStatusLine(TRect r) {
    r.a.y = r.b.y - 1;

    auto& appStatusDef = *new TStatusDef(0, 0xFFFF) + *new TStatusItem("~Ctrl+N~ New program", kbNoKey, cmNew) +
        *new TStatusItem("~Ctrl+O~ Open program", kbNoKey, cmOpen) + *new TStatusItem("~Ctrl+Q~ Exit", kbNoKey, cmQuit);

    auto& editorWindowStatusDef = *new TStatusDef(hcide_editorWindow, hcide_editorWindow) +
        *new TStatusItem("~Ctrl+P~ View program", kbNoKey, kCmdProgramContentsWindow) +
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

static char getPaletteColor(char* palette, size_t index) {
    return palette[index - 1];
}

static void updatePalette(char* appPalette, size_t appPaletteIndex, uint8_t mask, uint8_t newValue) {
    appPalette[appPaletteIndex - 1] = (appPalette[appPaletteIndex - 1] & ~mask) | newValue;
}

static void updatePalette(
    char* appPalette,
    char* windowPalette,
    char windowPaletteIndex,
    uint8_t mask,
    uint8_t newValue) {
    auto appIndex = static_cast<size_t>(windowPalette[windowPaletteIndex - 1]);
    appPalette[appIndex - 1] = (appPalette[appIndex - 1] & ~mask) | newValue;
}

TPalette& App::getPalette() const {
    static char array[256] = {};
    static auto isInitialized = false;

    if (!isInitialized) {
        // 0x08 - 0x0F: cpBlueWindow
        // 0x10 - 0x17: cpCyanWindow
        // 0x18 - 0x1F: cpGrayWindow
        // 0x20 - 0x3F: cpGrayDialog
        // 0x40 - 0x5F: cpBlueDialog
        // 0x60 - 0x7F: cpCyanDialog
        // 0x80 - 0x87: cHelpWindow
        memcpy(array, cpAppColor, sizeof(cpAppColor));

        // change the list viewer colors in blue/cyan dialogs
        updatePalette(array, cpBlueDialog, kPaletteListViewerActiveInactive, 0xFF, kBgColorBlue | kFgColorWhite);
        updatePalette(array, cpBlueDialog, kPaletteListViewerFocused, 0xFF, kBgColorGreen | kFgColorBrightWhite);
        updatePalette(array, cpBlueDialog, kPaletteListViewerSelected, 0xFF, kBgColorBlue | kFgColorYellow);
        updatePalette(array, cpBlueDialog, kPaletteListViewerDivider, 0xFF, kBgColorBlue | kFgColorGray);
        updatePalette(array, cpCyanDialog, kPaletteListViewerActiveInactive, 0xFF, kBgColorCyan | kFgColorBrightWhite);
        updatePalette(array, cpCyanDialog, kPaletteListViewerFocused, 0xFF, kBgColorGreen | kFgColorBrightWhite);
        updatePalette(array, cpCyanDialog, kPaletteListViewerSelected, 0xFF, kBgColorCyan | kFgColorYellow);
        updatePalette(array, cpCyanDialog, kPaletteListViewerDivider, 0xFF, kBgColorCyan | kFgColorWhite);

        // white text in editors instead of yellow
        updatePalette(array, cpBlueWindow, kPaletteEditorNormal, kFgColorMask, kFgColorWhite);

        // gray windows/dialogs: the default foreground doesn't have enough contrast.
        for (size_t i = 0x18; i <= 0x3F; i++) {
            auto color = getPaletteColor(array, i);
            if (color == (kFgColorBrightWhite | kBgColorWhite)) {
                updatePalette(array, i, kFgColorMask, kFgColorBlack);
            } else if (color == (kFgColorLightGreen | kBgColorWhite)) {
                updatePalette(array, i, kFgColorMask, kFgColorGreen);
            }
        }

        // use 0x80 - 0x100 to represent every possible combination of foreground and background colors. note that
        // we're trampling on the help viewer's palette at 0x80 - 0x87. also note that the foreground and background
        // colors are in the low 7 bits, so we can just mask off the MSB to get from the palette entry to the video
        // attribute byte.
        for (size_t i = 0x80; i <= 0x100; i++) {
            auto color = static_cast<char>(i & 0x7F);
            updatePalette(array, i, 0xFF, color);
        }

        // in order to cope with the help viewer problem, let's convert the help palette to these new custom entries
        // and then provide it in a place where our custom HelpWindow subclass can read it.
        for (size_t i = 0x80; i <= 0x87; i++) {
            auto color = getPaletteColor(cpAppColor, i);
            helpWindowPalette[i - 0x80] = 0x80 | color;
        }

        isInitialized = true;
    }

    static TPalette color(array, sizeof(array) - 1);
    static TPalette blackwhite(cpAppBlackWhite, sizeof(cpAppBlackWhite) - 1);
    static TPalette monochrome(cpAppMonochrome, sizeof(cpAppMonochrome) - 1);
    static TPalette* palettes[] = { &color, &blackwhite, &monochrome };
    return *(palettes[appPalette]);
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
            onProgramAddTextEditor(EditorType::kSubroutine);
            return true;

        case kCmdProgramAddFunction:
            onProgramAddTextEditor(EditorType::kFunction);
            return true;

        case kCmdProgramAddGlobalVariable:
            onProgramAddTextEditor(EditorType::kGlobalVariable);
            return true;

        case kCmdProgramAddConstant:
            onProgramAddTextEditor(EditorType::kConstant);
            return true;

        case kCmdProgramAddType:
            onProgramAddTextEditor(EditorType::kType);
            return true;

        case kCmdProgramAddForm:
            onProgramAddDesigner(DesignerType::kForm);
            return true;

        case kCmdProgramAddCustomControl:
            onProgramAddDesigner(DesignerType::kCustomControl);
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

        case kCmdProgramContentsWindow:
            onViewProgram();
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
    } else {
        sourceProgram->members.push_back(
            std::make_unique<SourceMember>(SourceMemberType::kProcedure, "sub main()\n\nend sub\n", 11, 11));
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
        e.window->select();
    } else {
        auto window = new EditorWindow(getNewWindowRect(82, 30), member, []() -> void {
            // onUpdated
            auto* programWindow = findProgramWindow(deskTop);
            if (programWindow) {
                programWindow->updateListItems();
            }
        });
        deskTop->insert(window);
    }
}

void App::showNewDesignerWindow(SourceMember* member) {
    // is there already a designer open for this member?
    FindDesignerWindowEventArgs e = { 0 };
    e.member = member;
    message(deskTop, evBroadcast, kCmdFindDesignerWindow, &e);
    if (e.window) {
        e.window->select();
    } else {
        auto window = new DesignerWindow(getNewWindowRect(51, 20), member, []() -> void {
            // onUpdated
            auto* programWindow = findProgramWindow(deskTop);
            if (programWindow) {
                programWindow->updateListItems();
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

void App::onProgramAddTextEditor(EditorType type) {
    auto* programWindow = findProgramWindow(deskTop);
    if (!programWindow) {
        return;
    }

    auto memberType = SourceMemberType::kProcedure;
    auto source = std::string();
    auto selectionStart = 0;
    auto selectionEnd = 0;

    switch (type) {
        case EditorType::kConstant:
            memberType = SourceMemberType::kGlobal;
            source = "const untitled = 0\n";
            selectionStart = 6;
            selectionEnd = 14;
            break;
        case EditorType::kGlobalVariable:
            memberType = SourceMemberType::kGlobal;
            source = "dim untitled as number\n";
            selectionStart = 4;
            selectionEnd = 12;
            break;
        case EditorType::kFunction:
            memberType = SourceMemberType::kProcedure;
            source = "function untitled() as integer\n\nend function\n";
            selectionStart = 9;
            selectionEnd = 17;
            break;
        case EditorType::kSubroutine:
            memberType = SourceMemberType::kProcedure;
            source = "sub untitled()\n\nend sub\n";
            selectionStart = 4;
            selectionEnd = 12;
            break;
        case EditorType::kType:
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

void App::onProgramAddDesigner(DesignerType type) {
    auto* programWindow = findProgramWindow(deskTop);
    if (!programWindow) {
        return;
    }

    auto source = std::string();

    switch (type) {
        case DesignerType::kForm:
            source = "form untitled\nend form\n";
            break;
        case DesignerType::kCustomControl:
            source = "control untitled\nend control\n";
            break;
        default:
            assert(false);
            break;
    }

    auto sourceMember = std::make_unique<SourceMember>(SourceMemberType::kDesign, source, 0, 0);
    auto* sourceMemberPtr = sourceMember.get();
    programWindow->addNewSourceMember(std::move(sourceMember));

    showNewDesignerWindow(sourceMemberPtr);
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
    auto helpWindow = new HelpWindow(helpFile, topic);
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

void App::onViewProgram() {
    auto* window = findProgramWindow(deskTop);
    if (window) {
        window->select();
    }
}

}  // namespace tmbasic

int main(int argc, char** argv) {
    tmbasic::App app(argc, argv);
    app.run();
    app.shutDown();
    return 0;
}
