#include "ProgramWindow.h"
#include "../../obj/resources/help/helpfile.h"
#include "../compiler/TargetPlatform.h"
#include "../compiler/compileProgram.h"
#include "../compiler/compressGz.h"
#include "../compiler/createTarArchive.h"
#include "../compiler/createZipArchive.h"
#include "../compiler/makeExeFile.h"
#include "../util/DialogPtr.h"
#include "../util/Frame.h"
#include "../util/Label.h"
#include "../util/ListViewer.h"
#include "../util/ScrollBar.h"
#include "../util/ViewPtr.h"
#include "../util/WindowPtr.h"
#include "../util/path.h"
#include "../util/tvutil.h"
#include "../vm/Program.h"
#include "CodeEditorWindow.h"
#include "DesignerWindow.h"
#include "GridLayout.h"
#include "PictureWindow.h"
#include "constants.h"
#include "events.h"

using compiler::SourceMember;
using compiler::SourceMemberType;
using compiler::SourceProgram;
using compiler::TargetPlatform;
using util::DialogPtr;
using util::Label;
using util::ListViewer;
using util::ViewPtr;
using util::WindowPtr;
using vm::Program;

namespace tmbasic {

class SourceMembersListBox : public util::ListViewer {
   public:
    SourceMembersListBox(
        const TRect& bounds,
        uint16_t numCols,
        util::ScrollBar* vScrollBar,
        const SourceProgram& program,
        std::function<void(SourceMember*)> onMemberOpen,
        std::function<void()> onEnableDisableCommands)
        : ListViewer(bounds, numCols, nullptr, vScrollBar),
          _program(program),
          _onMemberOpen(std::move(onMemberOpen)),
          _onEnableDisableCommands(std::move(onEnableDisableCommands)) {}

    SourceMember* getSelectedMember() {
        return focused >= 0 && static_cast<size_t>(focused) < _items.size() ? _items.at(focused) : nullptr;
    }

    void updateItems() {
        auto* selectedMember = getSelectedMember();

        _items.clear();

        for (const auto& member : _program.members) {
            _items.push_back(member.get());
        }

        std::sort(_items.begin(), _items.end(), [](const auto* lhs, const auto* rhs) {
            return lhs->identifier == rhs->identifier ? lhs->displayName < rhs->displayName
                                                      : lhs->identifier < rhs->identifier;
        });

        setRange(_items.size());

        focused = 0;
        for (size_t i = 0; i < _items.size(); i++) {
            if (_items.at(i) == selectedMember) {
                focused = i;
                break;
            }
        }

        drawView();

        _onEnableDisableCommands();
    }

    void getText(char* dest, int16_t item, int16_t maxLen) override {
        if (item >= 0 && static_cast<size_t>(item) < _items.size()) {
            strncpy(dest, _items.at(item)->displayName.c_str(), maxLen);
            dest[maxLen] = '\0';  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        } else {
            dest[0] = '\0';  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
    }

    void selectItem(int16_t item) override {
        openMember(item);
        ListViewer::selectItem(item);
    }

   private:
    void openMember(int16_t index) {
        if (index >= 0 && static_cast<size_t>(index) < _items.size()) {
            _onMemberOpen(_items.at(index));
        }
    }

    const SourceProgram& _program;
    std::vector<SourceMember*> _items;
    std::function<void(SourceMember*)> _onMemberOpen;
    std::function<void()> _onEnableDisableCommands;
};

class ProgramWindowPrivate {
   public:
    ProgramWindow* window;
    bool dirty{ false };
    std::optional<std::string> filePath;
    std::unique_ptr<vm::Program> vmProgram = std::make_unique<Program>();
    std::unique_ptr<SourceProgram> sourceProgram;
    SourceMembersListBox* contentsListBox{};
    std::function<void(SourceMember*)> openMember;

    ProgramWindowPrivate(
        ProgramWindow* window,
        std::unique_ptr<SourceProgram> sourceProgram,
        std::function<void(SourceMember*)> openMember,
        std::optional<std::string> filePath)
        : window(window),
          sourceProgram(std::move(sourceProgram)),
          openMember(std::move(openMember)),
          filePath(std::move(filePath)) {}
};

static void updateTitle(ProgramWindowPrivate* p) {
    std::ostringstream s;
    if (p->dirty) {
        s << kCharBullet;
    }
    if (p->filePath.has_value()) {
        s << util::getFileName(*p->filePath);
    } else {
        s << "Untitled";
    }
    s << " (Program)";

    delete[] p->window->title;  // NOLINT(cppcoreguidelines-owning-memory)
    p->window->title = newStr(s.str());
}

static void enableDisableCommands(ProgramWindowPrivate* p, bool enable) {
    TCommandSet cmds;
    if (p->contentsListBox->range > 0 && p->contentsListBox->focused >= 0) {
        cmds.enableCmd(cmClear);
    } else {
        TView::disableCommand(cmClear);
    }
    (enable ? TView::enableCommands : TView::disableCommands)(cmds);
}

ProgramWindow::ProgramWindow(
    const TRect& r,
    std::unique_ptr<SourceProgram> sourceProgram,
    std::optional<std::string> filePath,
    std::function<void(SourceMember*)> openMember)
    : TWindow(r, "Untitled - Program", wnNoNumber),
      TWindowInit(initFrame),
      _private(new ProgramWindowPrivate(this, std::move(sourceProgram), std::move(openMember), std::move(filePath))) {
    setState(sfShadow, false);

    TCommandSet ts;
    ts.enableCmd(cmSave);
    ts.enableCmd(cmSaveAs);
    enableCommands(ts);

    ViewPtr<util::ScrollBar> vScrollBar{ TRect{ size.x - 1, 1, size.x, size.y - 1 } };
    vScrollBar.addTo(this);

    auto contentsListBoxRect = getExtent();
    contentsListBoxRect.grow(-1, -1);
    ViewPtr<SourceMembersListBox> contentsListBox{ contentsListBoxRect,
                                                   1,
                                                   vScrollBar,
                                                   *_private->sourceProgram,
                                                   [this](auto* member) -> void { _private->openMember(member); },
                                                   [this]() -> void { enableDisableCommands(_private, true); } };
    _private->contentsListBox = contentsListBox;
    _private->contentsListBox->growMode = gfGrowHiX | gfGrowHiY;
    contentsListBox.addTo(this);

    updateTitle(_private);
    updateListItems();
}

ProgramWindow::~ProgramWindow() = default;

TPalette& ProgramWindow::getPalette() const {
    static auto palette = TPalette(cpGrayDialog, sizeof(cpGrayDialog) - 1);
    return palette;
}

uint16_t ProgramWindow::getHelpCtx() {
    return hcide_programWindow;
}

static bool save(ProgramWindowPrivate* p, const std::string& filePath) {
    try {
        p->sourceProgram->save(filePath);
        p->filePath = filePath;
        p->dirty = false;
        updateTitle(p);
        p->window->frame->drawView();
        return true;
    } catch (const std::system_error& ex) {
        std::ostringstream s;
        s << "Save failed: " << ex.what();
        messageBox(s.str(), mfInformation | mfOKButton);
        return false;
    } catch (...) {
        messageBox("Save failed.", mfInformation | mfOKButton);
        return false;
    }
}

static bool onSaveAs(ProgramWindowPrivate* p) {
    auto d = DialogPtr<TFileDialog>("*.bas", "Save Program As (.BAS)", "~N~ame", fdOKButton, 101);
    auto result = false;

    if (TProgram::deskTop->execView(d) != cmCancel) {
        auto fileName = std::array<char, MAXPATH>();
        d->getFileName(fileName.data());
        if (save(p, fileName.data())) {
            result = true;
        }
    }

    return result;
}

static bool onSave(ProgramWindowPrivate* p) {
    if (p->filePath.has_value()) {
        return save(p, *p->filePath);
    }
    return onSaveAs(p);
}

static void onDeleteItem(ProgramWindowPrivate* p) {
    auto* member = p->contentsListBox->getSelectedMember();
    if (member == nullptr) {
        return;
    }

    auto choice = messageBox(
        fmt::format("Are you sure you want to delete \"{}\"?", util::ellipsis(member->identifier, 25)),
        mfOKCancel | mfConfirmation);
    if (choice != cmOK) {
        return;
    }

    // if the item is open in an editor window, then close the window.
    switch (member->memberType) {
        case SourceMemberType::kDesign: {
            FindDesignerWindowEventArgs eventArgs{ member, nullptr };
            message(TProgram::deskTop, evBroadcast, kCmdFindDesignerWindow, &eventArgs);
            if (eventArgs.window != nullptr) {
                eventArgs.window->close();
            }
            break;
        }

        case SourceMemberType::kPicture: {
            FindPictureWindowEventArgs eventArgs{ member, nullptr };
            message(TProgram::deskTop, evBroadcast, kCmdFindPictureWindow, &eventArgs);
            if (eventArgs.window != nullptr) {
                eventArgs.window->close();
            }
            break;
        }

        case SourceMemberType::kGlobal:
        case SourceMemberType::kProcedure:
        case SourceMemberType::kType: {
            FindEditorWindowEventArgs eventArgs{ member, nullptr };
            message(TProgram::deskTop, evBroadcast, kCmdFindEditorWindow, &eventArgs);
            if (eventArgs.window != nullptr) {
                eventArgs.window->close();
            }
            break;
        }

        default:
            assert(false);
            break;
    }

    for (auto iter = p->sourceProgram->members.begin(); iter != p->sourceProgram->members.end(); ++iter) {
        if (member == iter->get()) {
            p->sourceProgram->members.erase(iter);
            break;
        }
    }

    p->dirty = true;
    updateTitle(p);
    p->window->frame->drawView();
    p->window->updateListItems();
}

void ProgramWindow::handleEvent(TEvent& event) {
    if (event.what == evBroadcast) {
        switch (event.message.command) {
            case kCmdProgramSave:
                onSave(_private);
                break;

            case kCmdProgramSaveAs:
                onSaveAs(_private);
                break;

            case kCmdAppExit:
                if (!preClose()) {
                    *static_cast<bool*>(event.message.infoPtr) = true;
                    clearEvent(event);
                }
                break;

            case kCmdFindProgramWindow:
                *static_cast<ProgramWindow**>(event.message.infoPtr) = this;
                clearEvent(event);
                break;
        }
    } else if (event.what == evCommand && event.message.command == cmClear) {
        onDeleteItem(_private);
        clearEvent(event);
    } else if (event.what == evKeyDown && event.keyDown.keyCode == kbEnter) {
        auto* member = _private->contentsListBox->getSelectedMember();
        if (member != nullptr) {
            _private->openMember(member);
        }
    }

    TWindow::handleEvent(event);
}

// true = close, false = stay open
bool ProgramWindow::preClose() {
    if (_private->dirty) {
        std::ostringstream s;
        s << "Save changes to \""
          << (_private->filePath.has_value() ? util::getFileName(*_private->filePath) : "Untitled") << "\"?";
        auto result = messageBox(s.str(), mfWarning | mfYesNoCancel);
        if (result == cmCancel) {
            return false;
        }
        if (result == cmYes) {
            if (!onSave(_private)) {
                return false;
            }
        }
    }

    return true;
}

void ProgramWindow::close() {
    if (preClose()) {
        // close all other program-related windows first
        message(owner, evBroadcast, kCmdCloseProgramRelatedWindows, nullptr);

        TCommandSet ts;
        ts.enableCmd(cmSave);
        ts.enableCmd(cmSaveAs);
        disableCommands(ts);

        TWindow::close();
    }
}

bool ProgramWindow::isDirty() const {
    return _private->dirty;
}

void ProgramWindow::addNewSourceMember(std::unique_ptr<SourceMember> sourceMember) {
    _private->sourceProgram->members.push_back(std::move(sourceMember));
    _private->contentsListBox->updateItems();
    _private->dirty = true;
    updateTitle(_private);
}

void ProgramWindow::updateListItems() {
    _private->contentsListBox->updateItems();
}

void ProgramWindow::redrawListItems() {
    _private->contentsListBox->drawView();
}

void ProgramWindow::setState(uint16_t aState, bool enable) {
    TWindow::setState(aState, enable);

    if (aState == sfActive) {
        enableDisableCommands(_private, enable);
    }
}

static void compilerErrorMessageBox(const compiler::CompilerResult& compilerResult) {
    messageBox(
        fmt::format(
            "Error in \"{}\"\nLn {}, Col {}\n{}", compilerResult.token.sourceMember->identifier,
            compilerResult.token.lineIndex + 1, compilerResult.token.columnIndex + 1, compilerResult.message),
        mfError | mfOKButton);
}

void ProgramWindow::checkForErrors() {
    compiler::CompiledProgram program;
    auto compilerResult = compiler::compileProgram(*_private->sourceProgram, &program);
    if (compilerResult.isSuccess) {
        messageBox("No errors!", mfInformation | mfOKButton);
    } else {
        compilerErrorMessageBox(compilerResult);
    }
}

static void publishPlatform(
    TargetPlatform platform,
    const std::string& programName,
    const std::vector<uint8_t>& pcode,
    const std::string& publishDir) {
    auto isZip = compiler::getTargetPlatformArchiveType(platform) == compiler::TargetPlatformArchiveType::kZip;
    auto archiveFilename =
        fmt::format("{}-{}.{}", programName, compiler::getPlatformName(platform), isZip ? "zip" : "tar.gz");
    auto archiveFilePath = util::pathCombine(publishDir, archiveFilename);
    auto exeData = compiler::makeExeFile(pcode, platform);
    auto exeFilename = fmt::format("{}{}", programName, compiler::getPlatformExeExtension(platform));
    const auto* licFilename = "LICENSE.txt";
    auto licString = compiler::getLicenseForPlatform(platform);
    std::vector<uint8_t> licData{};
    licData.insert(licData.end(), licString.begin(), licString.end());
    if (isZip) {
        std::vector<compiler::ZipEntry> entries{
            compiler::ZipEntry{ exeFilename, exeData },
            compiler::ZipEntry{ licFilename, licData },
        };
        compiler::createZipArchive(archiveFilePath, entries);
    } else {
        std::vector<compiler::TarEntry> entries{
            compiler::TarEntry{ exeFilename, exeData, 0777 },
            compiler::TarEntry{ licFilename, licData, 0664 },
        };
        auto tar = compiler::createTarArchive(entries);
        auto gz = compiler::compressGz(tar);
        std::ofstream f{ archiveFilePath, std::ios::out | std::ios::binary };
        f.write(reinterpret_cast<const char*>(gz.data()), gz.size());
    }
}

class PublishStatusWindow : public TWindow {
   public:
    ViewPtr<Label> labelTop{ TRect{ 0, 0, 30, 1 }, "Checking for errors..." };
    ViewPtr<Label> labelBottom{ TRect{ 0, 0, 30, 1 }, "" };
    PublishStatusWindow() : TWindow(TRect{ 0, 0, 0, 0 }, "Please Wait", wnNoNumber), TWindowInit(initFrame) {
        palette = wpGrayWindow;
        options |= ofCentered;
        flags &= ~(wfClose | wfZoom);
        GridLayout(
            1,
            {
                labelTop.take(),
                labelBottom.take(),
            })
            .setRowSpacing(0)
            .addTo(this);
        labelBottom->colorActive = { TColorBIOS{ 4 }, TColorBIOS{ 7 } };
    }
};

void ProgramWindow::publish() {
    if (!_private->filePath.has_value()) {
        messageBox("Please save your program first.", mfError | mfOKButton);
        return;
    }

    WindowPtr<PublishStatusWindow> statusWindow{};
    statusWindow.get()->drawView();
    TScreen::flushScreen();

    compiler::CompiledProgram program;
    auto compilerResult = compiler::compileProgram(*_private->sourceProgram, &program);
    if (!compilerResult.isSuccess) {
        statusWindow.get()->close();
        compilerErrorMessageBox(compilerResult);
        return;
    }

    auto basFilePath = *_private->filePath;
    auto basFilename = util::getFileName(basFilePath);
    auto publishDir = util::pathCombine(util::getDirectoryName(basFilePath), "publish");
    util::createDirectory(publishDir);
    auto programName = basFilename.size() > 4 ? basFilename.substr(0, basFilename.size() - 4) : basFilename;
    auto pcode = program.vmProgram.serialize();

    try {
        for (auto platform : compiler::getTargetPlatforms()) {
            statusWindow.get()->labelTop->setTitle("Publishing:");
            statusWindow.get()->labelTop->drawView();
            statusWindow.get()->labelBottom->setTitle(compiler::getPlatformName(platform));
            statusWindow.get()->labelBottom->drawView();
            TScreen::flushScreen();
            publishPlatform(platform, programName, pcode, publishDir);
        }
        statusWindow.get()->close();
        messageBox("Publish successful!", mfInformation | mfOKButton);

    } catch (const std::runtime_error& ex) {
        statusWindow.get()->close();
        messageBox(ex.what(), mfError | mfOKButton);
    }
}

}  // namespace tmbasic
