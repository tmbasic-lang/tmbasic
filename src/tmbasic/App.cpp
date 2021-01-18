#include "tmbasic/App.h"
#include "../../obj/resources/help/helpfile.h"
#include "compiler/SourceProgram.h"
#include "shared/util/membuf.h"
#include "tmbasic/AboutDialog.h"
#include "tmbasic/DesignerWindow.h"
#include "tmbasic/DialogPtr.h"
#include "tmbasic/EditorWindow.h"
#include "tmbasic/HelpWindow.h"
#include "tmbasic/ProgramWindow.h"
#include "tmbasic/Resource.h"
#include "tmbasic/WindowPtr.h"
#include "tmbasic/constants.h"
#include "tmbasic/events.h"

using compiler::SourceMember;
using compiler::SourceMemberType;
using compiler::SourceProgram;
using util::MemoryIopstream;

namespace tmbasic {

std::array<char, 9> App::helpWindowPalette = {};

App::App(int /*argc*/, char** /*argv*/)
    : TProgInit(initStatusLine, initMenuBar, TApplication::initDeskTop), _newWindowX(2), _newWindowY(1) {
    TCommandSet ts;
    ts.enableCmd(cmUndo);
    ts.enableCmd(kCmdEditRedo);
    ts.enableCmd(cmCut);
    ts.enableCmd(cmCopy);
    ts.enableCmd(cmPaste);
    ts.enableCmd(cmSave);
    ts.enableCmd(cmSaveAs);
    ts.enableCmd(kCmdDesignAddButton);
    ts.enableCmd(kCmdDesignAddCheckBox);
    ts.enableCmd(kCmdDesignAddGroupBox);
    ts.enableCmd(kCmdDesignAddLabel);
    ts.enableCmd(kCmdDesignAddListBox);
    ts.enableCmd(kCmdDesignAddRadioButton);
    ts.enableCmd(kCmdDesignAddScrollBar);
    ts.enableCmd(kCmdDesignAddTextBox);
    ts.enableCmd(kCmdDesignAddCustomControl);
    disableCommands(ts);

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

TMenuBar* App::initMenuBar(TRect r) {
    auto& fileMenu = *new TSubMenu("~F~ile", kbAltF) + *new TMenuItem("~N~ew", cmNew, kbCtrlN, hcNoContext, "Ctrl+N") +
        *new TMenuItem("~O~pen...", cmOpen, kbCtrlO, hcNoContext, "Ctrl+O") +
        *new TMenuItem("~S~ave", cmSave, kbCtrlS, hcNoContext, "Ctrl+S") +
        *new TMenuItem("S~a~ve as...", cmSaveAs, kbCtrlS) + newLine() +
        *new TMenuItem("E~x~it", cmQuit, kbCtrlQ, hcNoContext, "Ctrl+Q");

    auto& editMenu = *new TSubMenu("~E~dit", kbAltE) +
        *new TMenuItem("~U~ndo", cmUndo, kbCtrlZ, hcNoContext, "Ctrl+Z") +
        *new TMenuItem("~R~edo", kCmdEditRedo, kbCtrlY, hcNoContext, "Ctrl+Y") + newLine() +
        *new TMenuItem("Cu~t~", cmCut, kbCtrlX, hcNoContext, "Ctrl+X") +
        *new TMenuItem("~C~opy", cmCopy, kbCtrlC, hcNoContext, "Ctrl+C") +
        *new TMenuItem("~P~aste", cmPaste, kbCtrlV, hcNoContext, "Ctrl+V");

    auto& viewMenu = *new TSubMenu("~V~iew", kbAltE) +
        *new TMenuItem("~P~rogram", kCmdProgramContentsWindow, kbCtrlP, hcNoContext, "Ctrl+P");

    auto& programMenu = *new TSubMenu("~P~rogram", kbAltP) +
        *new TMenuItem("~R~un", kCmdProgramRun, kbCtrlR, hcNoContext, "Ctrl+R") + newLine() +
        *new TMenuItem("Add ~s~ubroutine", kCmdProgramAddSubroutine, kbNoKey) +
        *new TMenuItem("Add ~f~unction", kCmdProgramAddFunction, kbNoKey) +
        *new TMenuItem("Add ~g~lobal variable", kCmdProgramAddGlobalVariable, kbNoKey) +
        *new TMenuItem("Add ~c~onstant", kCmdProgramAddConstant, kbNoKey) +
        *new TMenuItem("Add ~t~ype", kCmdProgramAddType, kbNoKey) +
        *new TMenuItem("Add f~o~rm", kCmdProgramAddForm, kbNoKey) +
        *new TMenuItem("Add c~u~stom control", kCmdProgramAddCustomControl, kbNoKey);

    auto& designMenu = *new TSubMenu("~D~esign", kbAltD) +
        *new TMenuItem("Add ~b~utton", kCmdDesignAddButton, kbNoKey) +
        *new TMenuItem("Add ~c~heck box", kCmdDesignAddCheckBox, kbNoKey) +
        *new TMenuItem("Add ~g~roup box", kCmdDesignAddGroupBox, kbNoKey) +
        *new TMenuItem("Add ~l~abel", kCmdDesignAddLabel, kbNoKey) +
        *new TMenuItem("Add list bo~x~", kCmdDesignAddListBox, kbNoKey) +
        *new TMenuItem("Add ~r~adio button", kCmdDesignAddRadioButton, kbNoKey) +
        *new TMenuItem("Add ~s~croll bar", kCmdDesignAddScrollBar, kbNoKey) +
        *new TMenuItem("Add ~t~ext box", kCmdDesignAddTextBox, kbNoKey) + newLine() +
        *new TMenuItem("Add c~u~stom control", kCmdDesignAddCustomControl, kbNoKey);

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
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    return new TMenuBar(r, fileMenu + editMenu + viewMenu + programMenu + designMenu + windowMenu + helpMenu);
}

TStatusLine* App::initStatusLine(TRect r) {
    r.a.y = r.b.y - 1;

    auto& appStatusDef = *new TStatusDef(0, 0xFFFF);

    auto& editorWindowStatusDef = *new TStatusDef(hcide_editorWindow, hcide_editorWindow) +
        *new TStatusItem("~Ctrl+P~ View program", kbNoKey, kCmdProgramContentsWindow) +
        *new TStatusItem("~Ctrl+W~ Close editor", kbNoKey, cmClose);
    editorWindowStatusDef.next = &appStatusDef;

    auto& designerWindowStatusDef = *new TStatusDef(hcide_designerWindow, hcide_designerWindow) +
        *new TStatusItem("~Ctrl+P~ View program", kbNoKey, kCmdProgramContentsWindow) +
        *new TStatusItem("~Ctrl+W~ Close designer", kbNoKey, cmClose) + *new TStatusItem("~F10~ Menu", kbF10, cmMenu);
    designerWindowStatusDef.next = &editorWindowStatusDef;

    auto& programWindowStatusDef = *new TStatusDef(hcide_programWindow, hcide_programWindow) +
        *new TStatusItem("~Ctrl+S~ Save", kbNoKey, cmSave) + *new TStatusItem("~Ctrl+R~ Run", kbNoKey, kCmdProgramRun) +
        *new TStatusItem("~F10~ Menu", kbF10, cmMenu);
    programWindowStatusDef.next = &designerWindowStatusDef;

    return new TStatusLine(r, programWindowStatusDef);  // NOLINT(cppcoreguidelines-owning-memory)
}

static char getPaletteColor(const char* palette, size_t index) {
    return palette[index - 1];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

static void updatePalette(std::array<char, 256>* appPalette, size_t appPaletteIndex, uint8_t mask, uint8_t newValue) {
    appPalette->at(appPaletteIndex - 1) = (appPalette->at(appPaletteIndex - 1) & ~mask) | newValue;
}

static void updatePalette(
    std::array<char, 256>* appPalette,
    const char* windowPalette,
    char windowPaletteIndex,
    uint8_t mask,
    uint8_t newValue) {
    auto appIndex = static_cast<size_t>(
        windowPalette[windowPaletteIndex - 1]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    appPalette->at(appIndex - 1) = (appPalette->at(appIndex - 1) & ~mask) | newValue;
}

TPalette& App::getPalette() const {
    static auto array = std::array<char, 256>();
    static auto isInitialized = false;

    if (!isInitialized) {
        // 0x08 - 0x0F: cpBlueWindow
        // 0x10 - 0x17: cpCyanWindow
        // 0x18 - 0x1F: cpGrayWindow
        // 0x20 - 0x3F: cpGrayDialog
        // 0x40 - 0x5F: cpBlueDialog
        // 0x60 - 0x7F: cpCyanDialog
        // 0x80 - 0x87: cHelpWindow
        memcpy(array.data(), cpAppColor, sizeof(cpAppColor));

        // change the list viewer colors in blue/cyan dialogs
        updatePalette(&array, cpBlueDialog, kPaletteListViewerActiveInactive, 0xFF, kBgColorBlue | kFgColorWhite);
        updatePalette(&array, cpBlueDialog, kPaletteListViewerFocused, 0xFF, kBgColorGreen | kFgColorBrightWhite);
        updatePalette(&array, cpBlueDialog, kPaletteListViewerSelected, 0xFF, kBgColorBlue | kFgColorYellow);
        updatePalette(&array, cpBlueDialog, kPaletteListViewerDivider, 0xFF, kBgColorBlue | kFgColorGray);
        updatePalette(&array, cpCyanDialog, kPaletteListViewerActiveInactive, 0xFF, kBgColorCyan | kFgColorBrightWhite);
        updatePalette(&array, cpCyanDialog, kPaletteListViewerFocused, 0xFF, kBgColorGreen | kFgColorBrightWhite);
        updatePalette(&array, cpCyanDialog, kPaletteListViewerSelected, 0xFF, kBgColorCyan | kFgColorYellow);
        updatePalette(&array, cpCyanDialog, kPaletteListViewerDivider, 0xFF, kBgColorCyan | kFgColorWhite);

        // white text in editors instead of yellow
        updatePalette(&array, cpBlueWindow, kPaletteEditorNormal, kFgColorMask, kFgColorWhite);

        // gray windows/dialogs: the default foreground doesn't have enough contrast.
        for (size_t i = 0x18; i <= 0x3F; i++) {
            auto color = getPaletteColor(array.data(), i);
            if (color == (kFgColorBrightWhite | kBgColorWhite)) {
                updatePalette(&array, i, kFgColorMask, kFgColorBlack);
            } else if (color == (kFgColorLightGreen | kBgColorWhite)) {
                updatePalette(&array, i, kFgColorMask, kFgColorGreen);
            }
        }

        // use 0x80 - 0x100 to represent every possible combination of foreground and background colors. note that
        // we're trampling on the help viewer's palette at 0x80 - 0x87. also note that the foreground and background
        // colors are in the low 7 bits, so we can just mask off the MSB to get from the palette entry to the video
        // attribute byte.
        for (size_t i = 0x80; i <= 0x100; i++) {
            auto color = static_cast<char>(i & 0x7F);
            updatePalette(&array, i, 0xFF, color);
        }

        // in order to cope with the help viewer problem, let's convert the help palette to these new custom entries
        // and then provide it in a place where our custom HelpWindow subclass can read it.
        for (size_t i = 0x80; i <= 0x87; i++) {
            auto color = getPaletteColor(cpAppColor, i);
            helpWindowPalette.at(i - 0x80) = 0x80 | color;
        }

        isInitialized = true;
    }

    static TPalette color(array.data(), sizeof(array) - 1);
    static TPalette blackwhite(cpAppBlackWhite, sizeof(cpAppBlackWhite) - 1);
    static TPalette monochrome(cpAppMonochrome, sizeof(cpAppMonochrome) - 1);
    static std::array<TPalette*, 3> palettes = { &color, &blackwhite, &monochrome };
    return *palettes.at(appPalette);
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
            onHelpAbout();
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
    if (width > deskTop->size.x - 2) {
        width = deskTop->size.x - 2;
    }
    if (height > deskTop->size.y - 2) {
        height = deskTop->size.y - 2;
    }

    auto maxX = deskTop->size.x - width;
    auto maxY = deskTop->size.y - height;

    auto x = _newWindowX % maxX;
    auto y = _newWindowY % maxY;

    if (x < 1) {
        x = 1;
    }
    if (y < 1) {
        y = 1;
    }

    _newWindowX = x + 2;
    _newWindowY = y + 1;

    return TRect(x, y, x + width, y + height);
}

ProgramWindow* App::findProgramWindow(TDeskTop* deskTop) {
    ProgramWindow* programWindow = nullptr;
    message(deskTop, evBroadcast, kCmdFindProgramWindow, &programWindow);
    return programWindow;
}

bool App::closeProgramWindow(ProgramWindow* programWindow) {
    if (programWindow != nullptr) {
        programWindow->close();
        if (programWindow->frame != nullptr) {
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
        [this](auto* member) -> void { openEditorOrDesignerWindow(member); });
    deskTop->insert(window);
}

void App::openEditorOrDesignerWindow(compiler::SourceMember* member) {
    switch (member->memberType) {
        case compiler::SourceMemberType::kDesign:
            showDesignerWindow(member);
            break;

        default:
            showEditorWindow(member);
            break;
    }
}

void App::showEditorWindow(SourceMember* member) {
    // is there already an editor open for this member?
    FindEditorWindowEventArgs e = { nullptr };
    e.member = member;
    message(deskTop, evBroadcast, kCmdFindEditorWindow, &e);
    if (e.window != nullptr) {
        e.window->select();
    } else {
        auto* window = new EditorWindow(getNewWindowRect(82, 30), member, []() -> void {
            // onUpdated
            auto* programWindow = findProgramWindow(deskTop);
            if (programWindow != nullptr) {
                programWindow->updateListItems();
            }
        });
        deskTop->insert(window);
    }
}

void App::showDesignerWindow(SourceMember* member) {
    // is there already a designer open for this member?
    FindDesignerWindowEventArgs e = { nullptr };
    e.member = member;
    message(deskTop, evBroadcast, kCmdFindDesignerWindow, &e);
    if (e.window != nullptr) {
        e.window->select();
    } else {
        auto* window = new DesignerWindow(getNewWindowRect(51, 20), member, []() -> void {
            // onUpdated
            auto* programWindow = findProgramWindow(deskTop);
            if (programWindow != nullptr) {
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

    auto d = DialogPtr<TFileDialog>("*.bas", "Open Program (.BAS)", "~N~ame", fdOpenButton, 100);
    if (deskTop->execView(d.get()) != cmCancel) {
        if (!closeProgramWindow(programWindow)) {
            return;
        }

        auto fileName = std::array<char, MAXPATH>();
        d.get()->getFileName(fileName.data());
        showNewProgramWindow(fileName.data());
    }
}

void App::onProgramAddTextEditor(EditorType type) {
    auto* programWindow = findProgramWindow(deskTop);
    if (programWindow == nullptr) {
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

    showEditorWindow(sourceMemberPtr);
}

void App::onProgramAddDesigner(DesignerType type) {
    auto* programWindow = findProgramWindow(deskTop);
    if (programWindow == nullptr) {
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

    showDesignerWindow(sourceMemberPtr);
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
    try {
        auto* start = reinterpret_cast<char*>(kResourceHelp);  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto* end = reinterpret_cast<char*>(&kResourceHelp[kResourceHelp_len]);
        auto* stream = new MemoryIopstream(start, end);  // NOLINT(cppcoreguidelines-owning-memory)
        auto* helpFile = new THelpFile(*stream);         // NOLINT(cppcoreguidelines-owning-memory)
        auto* helpWindow = WindowPtr<HelpWindow>(helpFile, topic).get();
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
    } catch (std::runtime_error& ex) {
        messageBox(std::string("There was an error opening help: ") + ex.what(), mfError | mfOKButton);
        return;
    }
}

void App::onViewProgram() {
    auto* window = findProgramWindow(deskTop);
    if (window != nullptr) {
        window->select();
    }
}

void App::onHelpAbout() {
    auto dialog = DialogPtr<AboutDialog>();
    dialog.get()->options |= ofCentered;
    deskTop->execView(dialog.get());
}

}  // namespace tmbasic

int main(int argc, char** argv) {
    tmbasic::App app(argc, argv);
    app.run();
    app.shutDown();
    return 0;
}
