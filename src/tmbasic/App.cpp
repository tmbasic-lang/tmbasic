#include "tmbasic/App.h"
#include "../../obj/resources/help/helpfile.h"
#include "../compiler/SourceProgram.h"
#include "../util/DialogPtr.h"
#include "../util/StatusLine.h"
#include "../util/WindowPtr.h"
#include "../util/ViewPtr.h"
#include "../util/membuf.h"
#include "AboutDialog.h"
#include "DesignerWindow.h"
#include "EditorWindow.h"
#include "HelpWindow.h"
#include "ProgramWindow.h"
#include "Resource.h"
#include "constants.h"
#include "events.h"

using compiler::SourceMember;
using compiler::SourceMemberType;
using compiler::SourceProgram;
using util::DialogPtr;
using util::MemoryIopstream;
using util::StatusLine;
using util::ViewPtr;
using util::WindowPtr;

namespace tmbasic {

std::array<char, 9> App::helpWindowPalette = {};

// these are used to pass stuff from initMenuBar()/initStatusLine() to the App constructor
static PictureWindowStatusItems _newestPictureWindowStatusItems;
TStatusItem* App::insertColorDialogHelpStatusItem;

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
    ts.enableCmd(kCmdEditInsertColor);
    ts.enableCmd(kCmdEditInsertSymbol);
    ts.enableCmd(kCmdDesignAddButton);
    ts.enableCmd(kCmdDesignAddCheckBox);
    ts.enableCmd(kCmdDesignAddGroupBox);
    ts.enableCmd(kCmdDesignAddLabel);
    ts.enableCmd(kCmdDesignAddListBox);
    ts.enableCmd(kCmdDesignAddRadioButton);
    ts.enableCmd(kCmdDesignAddScrollBar);
    ts.enableCmd(kCmdDesignAddTextBox);
    ts.enableCmd(kCmdDesignAddCustomControl);
    ts.enableCmd(kCmdPictureCharacter);
    ts.enableCmd(kCmdPictureFg);
    ts.enableCmd(kCmdPictureBg);
    ts.enableCmd(kCmdPictureSelect);
    ts.enableCmd(kCmdPictureDraw);
    ts.enableCmd(kCmdPicturePick);
    ts.enableCmd(kCmdPictureType);
    ts.enableCmd(kCmdPictureMask);
    ts.enableCmd(kCmdPictureOptions);
    ts.enableCmd(kCmdPictureClear);
    ts.enableCmd(kCmdPicturePasteOk);
    ts.enableCmd(kCmdPicturePasteCancel);
    disableCommands(ts);

    onFileNew();

    _pictureWindowStatusItems = _newestPictureWindowStatusItems;
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
        *new TMenuItem("~P~aste", cmPaste, kbCtrlV, hcNoContext, "Ctrl+V") + newLine() +
        *new TMenuItem("Insert color...", kCmdEditInsertColor, kbNoKey) +
        *new TMenuItem("Insert ~s~ymbol...", kCmdEditInsertSymbol, kbNoKey);

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
        *new TMenuItem("Add c~u~stom control", kCmdProgramAddCustomControl, kbNoKey) +
        *new TMenuItem("Add ~p~icture", kCmdProgramAddPicture, kbNoKey);

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

    auto& pictureMenu = *new TSubMenu("P~i~cture", kbAltI) +
        *new TMenuItem("~F~oreground color", kCmdPictureFg, kbF1, hcNoContext, "F1") +
        *new TMenuItem("~B~ackground color", kCmdPictureBg, kbF2, hcNoContext, "F2") +
        *new TMenuItem("~C~haracter", kCmdPictureCharacter, kbF3, hcNoContext, "F3") + newLine() +
        *new TMenuItem("~S~elect tool", kCmdPictureSelect, kbF4, hcNoContext, "F4") +
        *new TMenuItem("~D~raw tool", kCmdPictureDraw, kbF5, hcNoContext, "F5") +
        *new TMenuItem("~P~ick tool", kCmdPicturePick, kbF6, hcNoContext, "F6") +
        *new TMenuItem("~T~ype tool", kCmdPictureType, kbF7, hcNoContext, "F7") +
        *new TMenuItem("~M~ask tool", kCmdPictureMask, kbF8, hcNoContext, "F8") + newLine() +
        *new TMenuItem("C~l~ear", kCmdPictureClear, kbDel, hcNoContext, "Del") + newLine() +
        *new TMenuItem("~O~ptions", kCmdPictureOptions, kbF9, hcNoContext, "F9");

    auto& windowMenu = *new TSubMenu("~W~indow", kbAltW) +
        *new TMenuItem("~S~ize/move", cmResize, kbCtrlF5, hcNoContext, "Ctrl+F5") +
        *new TMenuItem("~Z~oom", cmZoom, kbNoKey) + *new TMenuItem("~N~ext", cmNext, kbF6, hcNoContext, "F6") +
        *new TMenuItem("~P~revious", cmPrev, kbShiftF6, hcNoContext, "Shift+F6") +
        *new TMenuItem("~C~lose", cmClose, kbCtrlW, hcNoContext, "Ctrl+W");

    auto& helpMenu = *new TSubMenu("~H~elp", kbAltH) +
        *new TMenuItem("~D~ocumentation", kCmdHelpDocumentation, kbNoKey) +
        *new TMenuItem("~A~bout TMBASIC", kCmdHelpAbout, kbNoKey);

    r.b.y = r.a.y + 1;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    return new TMenuBar(
        r, fileMenu + editMenu + viewMenu + programMenu + designMenu + pictureMenu + windowMenu + helpMenu);
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

    ViewPtr<TStatusItem> fg{ "~F1~ FG", kbF1, kCmdPictureFg };
    ViewPtr<TStatusItem> bg{ "~F2~ BG", kbF2, kCmdPictureBg };
    ViewPtr<TStatusItem> character{ "~F3~  ", kbF3, kCmdPictureCharacter };
    ViewPtr<TStatusItem> select{ "~F4~ Select", kbF4, kCmdPictureSelect };
    ViewPtr<TStatusItem> draw{ "~F5~ Draw", kbF5, kCmdPictureDraw };
    ViewPtr<TStatusItem> pick{ "~F6~ Pick", kbF6, kCmdPicturePick };
    ViewPtr<TStatusItem> text{ "~F7~ Type", kbF7, kCmdPictureType };
    ViewPtr<TStatusItem> mask{ "~F8~ Mask", kbF8, kCmdPictureMask };
    ViewPtr<TStatusItem> options{ "~F9~ Options", kbF9, kCmdPictureOptions };
    auto& s = _newestPictureWindowStatusItems;
    s.fg = fg;
    s.bg = bg;
    s.character = character;
    s.select = select;
    s.draw = draw;
    s.pick = pick;
    s.text = text;
    s.mask = mask;
    s.options = options;
    auto& pictureWindowStatusDef = *new TStatusDef(hcide_pictureWindow, hcide_pictureWindow) + *fg.take() + *bg.take() +
        *character.take() + *select.take() + *draw.take() + *pick.take() + *text.take() + *mask.take() +
        *options.take();
    pictureWindowStatusDef.next = &programWindowStatusDef;

    ViewPtr<TStatusItem> insertColorHelp{ "~F1~ Help", kbF1, cmHelp };
    insertColorDialogHelpStatusItem = insertColorHelp;
    auto& insertColorDialogStatusDef =
        *new TStatusDef(hcide_insertColorDialog, hcide_insertColorDialog) + *insertColorHelp.take();
    insertColorDialogStatusDef.next = &pictureWindowStatusDef;

    auto* statusLine = new StatusLine(r, insertColorDialogStatusDef);  // NOLINT(cppcoreguidelines-owning-memory)
    s.statusLine = statusLine;
    s.characterColor = statusLine->addStatusItemColors(s.character);
    s.fgColor = statusLine->addStatusItemColors(s.fg);
    s.bgColor = statusLine->addStatusItemColors(s.bg);
    s.selectColor = statusLine->addStatusItemColors(s.select);
    s.drawColor = statusLine->addStatusItemColors(s.draw);
    s.pickColor = statusLine->addStatusItemColors(s.pick);
    s.textColor = statusLine->addStatusItemColors(s.text);
    s.maskColor = statusLine->addStatusItemColors(s.mask);
    s.optionsColor = statusLine->addStatusItemColors(s.options);
    return statusLine;
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

        case kCmdProgramAddPicture:
            onProgramAddPicture();
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

        case kCmdPictureCharacter:
        case kCmdPictureFg:
        case kCmdPictureBg:
        case kCmdPictureSelect:
        case kCmdPictureDraw:
        case kCmdPicturePick:
        case kCmdPictureType:
        case kCmdPictureMask:
        case kCmdPictureOptions:
            if (_pictureWindow != nullptr) {
                _pictureWindow->onStatusLineCommand(event->message.command);
            }
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
            std::make_unique<SourceMember>(SourceMemberType::kProcedure, "sub Main()\n\nend sub\n", 11, 11));
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

        case compiler::SourceMemberType::kPicture:
            showPictureWindow(member);
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
        if (deskTop->size.x < 82 && deskTop->size.y < 30) {
            window->zoom();
        }
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

void App::showPictureWindow(SourceMember* member) {
    // is there already a designer open for this member?
    FindPictureWindowEventArgs e = { nullptr };
    e.member = member;
    message(deskTop, evBroadcast, kCmdFindPictureWindow, &e);
    if (e.window != nullptr) {
        e.window->select();
    } else {
        auto* window = new PictureWindow(
            getNewWindowRect(90, 31), member,
            []() -> void {
                // onUpdated
                auto* programWindow = findProgramWindow(deskTop);
                if (programWindow != nullptr) {
                    programWindow->updateListItems();
                }
            },
            _pictureWindowStatusItems);
        deskTop->insert(window);
        if (deskTop->size.x < 90 && deskTop->size.y < 31) {
            window->zoom();
        }
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
            source = "dim untitled as Number\n";
            selectionStart = 4;
            selectionEnd = 12;
            break;
        case EditorType::kFunction:
            memberType = SourceMemberType::kProcedure;
            source = "function Untitled() as Number\n\nend function\n";
            selectionStart = 9;
            selectionEnd = 17;
            break;
        case EditorType::kSubroutine:
            memberType = SourceMemberType::kProcedure;
            source = "sub Untitled()\n\nend sub\n";
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
            source = "form Untitled\nend form\n";
            break;
        case DesignerType::kCustomControl:
            source = "control Untitled\nend control\n";
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

void App::onProgramAddPicture() {
    auto* programWindow = findProgramWindow(deskTop);
    if (programWindow == nullptr) {
        return;
    }

    std::string source = "picture Untitled\nend picture\n";
    auto sourceMember = std::make_unique<SourceMember>(SourceMemberType::kPicture, source, 0, 0);
    auto* sourceMemberPtr = sourceMember.get();
    programWindow->addNewSourceMember(std::move(sourceMember));

    showPictureWindow(sourceMemberPtr);
}

void App::onHelpDocumentation() {
    openHelpTopic(hcdoc);
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

void App::setPictureWindow(PictureWindow* pictureWindow) {
    _pictureWindow = pictureWindow;
}

}  // namespace tmbasic

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleTitle(TEXT("TMBASIC"));
#else
    setenv("LANG", "en_US.UTF-8", 1);
#endif
    tmbasic::App app(argc, argv);
    app.run();
    app.shutDown();
    return 0;
}
