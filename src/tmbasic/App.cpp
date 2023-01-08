#include "tmbasic/App.h"
#include "../../obj/resources/help/helpfile.h"
#include "../compiler/SourceProgram.h"
#include "../util/DialogPtr.h"
#include "../util/StatusLine.h"
#include "../util/ViewPtr.h"
#include "../util/WindowPtr.h"
#include "../util/membuf.h"
#include "AboutDialog.h"
#include "AddProgramItemDialog.h"
#include "DesignerWindow.h"
#include "CodeEditorWindow.h"
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

class Background : public TBackground {
   public:
    explicit Background(const TRect& bounds) : TBackground(bounds, ' ') {}
    TColorAttr mapColor(uchar /*index*/) noexcept override { return 0x88; }
};

class DeskTop : public TDeskTop {
   public:
    explicit DeskTop(const TRect& r) : TDeskTop(r), TDeskInit(initTmBackground) {}

    static TBackground* initTmBackground(TRect r) { return new Background(r); }
};

class AppPrivate {
   public:
    App* app{};
    int newWindowX = 2;
    int newWindowY = 1;
    std::chrono::steady_clock::time_point lastTimerTick;
    PictureWindowStatusItems pictureWindowStatusItems{};
    PictureWindow* pictureWindow = nullptr;

    static TDeskTop* initDeskTop(TRect r) {
        r.a.y++;
        r.b.y--;
        return new DeskTop(r);
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
            *new TMenuItem("~R~edo", kCmdEditRedo, kbCtrlY, hcNoContext, "Ctrl+Y") + newLine() +
            *new TMenuItem("Cu~t~", cmCut, kbCtrlX, hcNoContext, "Ctrl+X") +
            *new TMenuItem("~C~opy", cmCopy, kbCtrlC, hcNoContext, "Ctrl+C") +
            *new TMenuItem("~P~aste", cmPaste, kbCtrlV, hcNoContext, "Ctrl+V") + newLine() +
            *new TMenuItem("~D~elete", cmClear, kbCtrlD, hcNoContext, "Ctrl+D");

        auto& programMenu = *new TSubMenu("~P~rogram", kbAltP) +
            *new TMenuItem("~N~ew item...", kCmdProgramAddItem, kbNoKey) +
            *new TMenuItem("~I~mport from .BAS...", kCmdProgramImportItem, kbNoKey) + newLine() +
            *new TMenuItem("~R~un", kCmdProgramRun, kbCtrlR, hcNoContext, "Ctrl+R") + newLine() +
            *new TMenuItem("~C~heck for errors", kCmdProgramCheckForErrors, kbCtrlE, hcNoContext, "Ctrl+E") +
            *new TMenuItem("~P~ublish", kCmdProgramPublish, kbNoKey, hcNoContext);

        auto& windowMenu = *new TSubMenu("~W~indow", kbAltW) +
            *new TMenuItem("Program ~w~indow", kCmdProgramContentsWindow, kbCtrlP, hcNoContext, "Ctrl+P") + newLine() +
            *new TMenuItem("~S~ize/move", cmResize, kbAltDown, hcNoContext, "Alt+↓") +
            *new TMenuItem("~Z~oom", cmZoom, kbAltUp, hcNoContext, "Alt+↑") +
            *new TMenuItem("~N~ext", cmNext, kbAltRight, hcNoContext, "Alt+→") +
            *new TMenuItem("~P~revious", cmPrev, kbAltLeft, hcNoContext, "Alt+←") + newLine() +
            *new TMenuItem("~C~lose", cmClose, kbCtrlW, hcNoContext, "Ctrl+W");

        auto& helpMenu = *new TSubMenu("~H~elp", kbAltH) +
            *new TMenuItem("~D~ocumentation", kCmdHelpDocumentation, kbNoKey) +
            *new TMenuItem("~L~icense", kCmdHelpLicense, kbNoKey) + newLine() +
            *new TMenuItem("~A~bout TMBASIC", kCmdHelpAbout, kbNoKey);

        r.b.y = r.a.y + 1;
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return new TMenuBar(r, fileMenu + editMenu + programMenu + windowMenu + helpMenu);
    }

    static TStatusLine* initStatusLine(TRect r) {
        r.a.y = r.b.y - 1;

        auto& appStatusDef = *new TStatusDef(0, 0xFFFF);

        auto& editorWindowStatusDef = *new TStatusDef(hcide_editorWindow, hcide_editorWindow) +
            *new TStatusItem("~F1~ Insert color", kbF1, kCmdEditInsertColor) +
            *new TStatusItem("~F2~ Insert symbol", kbF2, kCmdEditInsertSymbol);

        editorWindowStatusDef.next = &appStatusDef;

        auto& designerWindowStatusDef = *new TStatusDef(hcide_designerWindow, hcide_designerWindow);
        designerWindowStatusDef.next = &editorWindowStatusDef;

        auto& programWindowStatusDef = *new TStatusDef(hcide_programWindow, hcide_programWindow) +
            *new TStatusItem("~F1~ New item", kbF1, kCmdProgramAddItem) +
            *new TStatusItem("~F2~ Import from .BAS", kbF2, kCmdProgramImportItem) +
            *new TStatusItem("~Ctrl+D~ Delete item", kbCtrlD, cmClear);
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
        auto& pictureWindowStatusDef = *new TStatusDef(hcide_pictureWindow, hcide_pictureWindow) + *fg.take() +
            *bg.take() + *character.take() + *select.take() + *draw.take() + *pick.take() + *text.take() +
            *mask.take() + *options.take();
        pictureWindowStatusDef.next = &programWindowStatusDef;

        ViewPtr<TStatusItem> insertColorHelp{ "~F1~ Help", kbF1, cmHelp };
        App::insertColorDialogHelpStatusItem = insertColorHelp;
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

    bool handleCommand(TEvent* event) {
        switch (event->message.command) {
            case cmNew:
                onFileNew();
                return true;

            case cmOpen:
                onFileOpen();
                return true;

            case cmSave:
                message(TApplication::deskTop, evBroadcast, kCmdUpdateSourceMembers, nullptr);
                findProgramWindow()->save();
                return true;

            case cmSaveAs:
                message(TApplication::deskTop, evBroadcast, kCmdUpdateSourceMembers, nullptr);
                findProgramWindow()->saveAs();
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

            case kCmdHelpLicense:
                app->openHelpTopic(hclicense);
                return true;

            case kCmdHelpAbout:
                onHelpAbout();
                return true;

            case cmQuit: {
                bool cancel = false;
                message(TApplication::deskTop, evBroadcast, kCmdAppExit, &cancel);
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
                if (pictureWindow != nullptr) {
                    pictureWindow->onStatusLineCommand(event->message.command);
                }
                return true;

            case kCmdProgramAddItem: {
                DialogPtr<AddProgramItemDialog> d{};
                auto cmd = TApplication::deskTop->execView(d.get());
                if (cmd != cmCancel) {
                    TEvent addEvent{ evCommand };
                    addEvent.message.command = cmd;
                    handleCommand(&addEvent);
                }
                return true;
            }

            case kCmdProgramCheckForErrors:
                message(TApplication::deskTop, evBroadcast, kCmdUpdateSourceMembers, nullptr);
                onProgramCheckForErrors();
                return true;

            case kCmdProgramPublish:
                message(TApplication::deskTop, evBroadcast, kCmdUpdateSourceMembers, nullptr);
                onProgramPublish();
                return true;

            case kCmdProgramRun:
                message(TApplication::deskTop, evBroadcast, kCmdUpdateSourceMembers, nullptr);
                onProgramRun();
                return true;

            default:
                return false;
        }
    }

    TRect getNewWindowRect(int width, int height) {
        auto* deskTop = TApplication::deskTop;
        if (width > deskTop->size.x - 2) {
            width = deskTop->size.x - 2;
        }
        if (height > deskTop->size.y - 2) {
            height = deskTop->size.y - 2;
        }

        auto maxX = deskTop->size.x - width;
        auto maxY = deskTop->size.y - height;

        auto x = newWindowX % maxX;
        auto y = newWindowY % maxY;

        if (x < 1) {
            x = 1;
        }
        if (y < 1) {
            y = 1;
        }

        newWindowX = x + 2;
        newWindowY = y + 1;

        return TRect(x, y, x + width, y + height);
    }

    static ProgramWindow* findProgramWindow() {
        ProgramWindow* programWindow = nullptr;
        message(TApplication::deskTop, evBroadcast, kCmdFindProgramWindow, &programWindow);
        return programWindow;
    }

    static bool closeProgramWindow(ProgramWindow* programWindow) {
        if (programWindow != nullptr) {
            programWindow->close();
            if (programWindow->frame != nullptr) {
                // the program window is still open, so the user must have clicked cancel
                return false;
            }
        }
        return true;
    }

    void onFileNew() {
        // close the existing program if there is one
        auto* programWindow = findProgramWindow();
        if (!closeProgramWindow(programWindow)) {
            return;
        }

        showNewProgramWindow(std::optional<std::string>());
    }

    void onFileOpen() {
        auto* programWindow = findProgramWindow();

        auto d = DialogPtr<TFileDialog>("*.bas", "Open Program (.BAS)", "~N~ame", fdOpenButton, 100);
        if (app->execView(d.get()) != cmCancel) {
            if (!closeProgramWindow(programWindow)) {
                return;
            }

            auto fileName = std::array<char, MAXPATH>();
            d.get()->getFileName(fileName.data());
            showNewProgramWindow(fileName.data());
        }
    }

    static void onViewProgram() {
        auto* window = findProgramWindow();
        if (window != nullptr) {
            window->select();
        }
    }

    void onProgramAddTextEditor(EditorType type) {
        auto* programWindow = findProgramWindow();
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
                source = "const kUntitled = 0\n";
                selectionStart = 6;
                selectionEnd = 15;
                break;
            case EditorType::kGlobalVariable:
                memberType = SourceMemberType::kGlobal;
                source = "dim shared Untitled as Number\n";
                selectionStart = 11;
                selectionEnd = 19;
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

    void onProgramAddDesigner(DesignerType type) {
        auto* programWindow = findProgramWindow();
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

    void onProgramAddPicture() {
        auto* programWindow = findProgramWindow();
        if (programWindow == nullptr) {
            return;
        }

        std::string source = "picture Untitled\nend picture\n";
        auto sourceMember = std::make_unique<SourceMember>(SourceMemberType::kPicture, source, 0, 0);
        auto* sourceMemberPtr = sourceMember.get();
        programWindow->addNewSourceMember(std::move(sourceMember));

        showPictureWindow(sourceMemberPtr);
    }

    void onHelpDocumentation() const { app->openHelpTopic(hcdoc); }

    static void onHelpAbout() {
        auto dialog = DialogPtr<AboutDialog>();
        dialog.get()->options |= ofCentered;
        TApplication::application->execView(dialog.get());
    }

    static TRect centeredRect(int width, int height) {
        auto x = (TApplication::deskTop->size.x - width) / 2;
        auto y = (TApplication::deskTop->size.y - height) / 2;
        return TRect(x, y, x + width, y + height);
    }

    void showNewProgramWindow(std::optional<std::string> filePath) {
        auto* deskTop = TProgram::deskTop;
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
            TRect(deskTop->size.x - 30, 0, deskTop->size.x, deskTop->size.y), std::move(sourceProgram), filePath,
            [this](auto* member) -> void { openEditorOrDesignerWindow(member); });
        deskTop->insert(window);
    }

    void openEditorOrDesignerWindow(compiler::SourceMember* member) {
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

    static void loadText(turbo::TScintilla* scintilla, std::string_view text) {
        // Allocate 1000 extra bytes, as in SciTE.
        turbo::call(*scintilla, SCI_ALLOCATE, text.size() + 1000, 0U);
        turbo::call(*scintilla, SCI_APPENDTEXT, text.size(), reinterpret_cast<sptr_t>(text.data()));
    }

    static std::unique_ptr<turbo::Editor> createEditor(compiler::SourceMember* member) {
        auto& scintilla = turbo::createScintilla();
        loadText(&scintilla, member->source);
        return std::make_unique<turbo::Editor>(scintilla);
    }

    void showEditorWindow(compiler::SourceMember* member) {
        // is there already an editor open for this member?
        FindEditorWindowEventArgs e = { nullptr };
        e.member = member;
        auto* deskTop = TApplication::deskTop;
        message(deskTop, evBroadcast, kCmdFindEditorWindow, &e);
        if (e.window != nullptr) {
            e.window->select();
        } else {
            auto editor = createEditor(member);
            auto* window = new CodeEditorWindow(getNewWindowRect(75, 20), std::move(editor), member, []() -> void {
                // onUpdated
                auto* programWindow = findProgramWindow();
                if (programWindow != nullptr) {
                    programWindow->setDirty();
                    programWindow->updateListItems();
                }
            });
            deskTop->insert(window);

            // workaround for the editor not showing line numbers and syntax coloring on first draw. this is not the
            // best fix but it works for now.
            window->zoom();
            window->zoom();
        }
    }

    void showDesignerWindow(compiler::SourceMember* member) {
        // is there already a designer open for this member?
        FindDesignerWindowEventArgs e = { nullptr };
        e.member = member;
        message(TApplication::deskTop, evBroadcast, kCmdFindDesignerWindow, &e);
        if (e.window != nullptr) {
            e.window->select();
        } else {
            auto* window = new DesignerWindow(getNewWindowRect(51, 20), member, []() -> void {
                // onUpdated
                auto* programWindow = findProgramWindow();
                if (programWindow != nullptr) {
                    programWindow->setDirty();
                    programWindow->updateListItems();
                }
            });
            TApplication::deskTop->insert(window);
        }
    }

    void showPictureWindow(compiler::SourceMember* member) {
        // is there already a designer open for this member?
        FindPictureWindowEventArgs e = { nullptr };
        e.member = member;
        message(TApplication::deskTop, evBroadcast, kCmdFindPictureWindow, &e);
        if (e.window != nullptr) {
            e.window->select();
        } else {
            auto* window = new PictureWindow(
                getNewWindowRect(80, 25), member,
                []() -> void {
                    // onUpdated
                    auto* programWindow = findProgramWindow();
                    if (programWindow != nullptr) {
                        programWindow->setDirty();
                        programWindow->updateListItems();
                    }
                },
                pictureWindowStatusItems);
            TApplication::deskTop->insert(window);
            if (TApplication::deskTop->size.x < 85 && TApplication::deskTop->size.y < 29) {
                window->zoom();
            }
        }
    }

    static void onProgramCheckForErrors() {
        auto* programWindow = findProgramWindow();
        if (!programWindow) {
            return;
        }

        programWindow->checkForErrors();
    }

    static void onProgramPublish() {
        auto* programWindow = findProgramWindow();
        if (!programWindow) {
            return;
        }

        programWindow->publish();
    }

    static void onProgramRun() {
        auto* programWindow = findProgramWindow();
        if (!programWindow) {
            return;
        }

        programWindow->run();
    }
};

App::App(int /*argc*/, char** /*argv*/)
    : TProgInit(AppPrivate::initStatusLine, AppPrivate::initMenuBar, AppPrivate::initDeskTop),
      _private(new AppPrivate()) {
    TCommandSet ts;
    ts.enableCmd(cmUndo);
    ts.enableCmd(kCmdEditRedo);
    ts.enableCmd(cmCut);
    ts.enableCmd(cmCopy);
    ts.enableCmd(cmPaste);
    ts.enableCmd(cmSave);
    ts.enableCmd(cmSaveAs);
    ts.enableCmd(cmClear);
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
    ts.enableCmd(kCmdPicturePasteOk);
    ts.enableCmd(kCmdPicturePasteCancel);
    ts.enableCmd(kCmdProgramAddItem);
    ts.enableCmd(kCmdProgramImportItem);
    ts.enableCmd(kCmdProgramRun);
    ts.enableCmd(kCmdProgramCheckForErrors);
    ts.enableCmd(kCmdProgramPublish);
    ts.enableCmd(kCmdProgramContentsWindow);
    disableCommands(ts);

    _private->app = this;
    _private->onFileNew();

    _private->pictureWindowStatusItems = _newestPictureWindowStatusItems;
}

App::~App() {
    delete _private;
}

void App::idle() {
    TApplication::idle();

    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> msec = now - _private->lastTimerTick;
    if (msec.count() >= 250) {
        _private->lastTimerTick = now;
        message(deskTop, evBroadcast, kCmdTimerTick, nullptr);
    }
}

void App::handleEvent(TEvent& event) {
    if (event.what == evCommand && _private->handleCommand(&event)) {
        clearEvent(event);
    }
    TApplication::handleEvent(event);
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

void App::openHelpTopic(uint16_t topic) {
    try {
        auto* start = reinterpret_cast<char*>(kResourceHelp);
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
        auto rect = _private->getNewWindowRect(width, height);
        helpWindow->locate(rect);
        if (TScreen::screenWidth < 95) {
            helpWindow->zoom();
        }
    } catch (std::runtime_error& ex) {
        messageBox(std::string("There was an error opening help: ") + ex.what(), mfError | mfOKButton);
        return;
    }
}

void App::setPictureWindow(PictureWindow* pictureWindow) {
    _private->pictureWindow = pictureWindow;
}

}  // namespace tmbasic

static void go(int argc, char** argv) {
    tmbasic::App app(argc, argv);
    app.run();
    app.shutDown();
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleTitle(TEXT("TMBASIC"));
#else
    setlocale(LC_ALL, "en_US.UTF-8");
#endif
    go(argc, argv);
    return 0;
}
