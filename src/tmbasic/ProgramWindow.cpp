#include "ProgramWindow.h"
#include "compiler/compileProgram.h"
#include "compiler/CompilerException.h"
#include "compiler/Publisher.h"
#include "compiler/TargetPlatform.h"
#include "shared/DialogPtr.h"
#include "shared/Frame.h"
#include "shared/Label.h"
#include "shared/ListViewer.h"
#include "shared/path.h"
#include "shared/ScrollBar.h"
#include "shared/tvutil.h"
#include "shared/ViewPtr.h"
#include "shared/WindowPtr.h"
#include "tmbasic/CodeEditorWindow.h"
#include "tmbasic/constants.h"
#include "tmbasic/DesignerWindow.h"
#include "tmbasic/events.h"
#include "tmbasic/helpfile.h"
#include "tmbasic/GridLayout.h"
#include "tmbasic/PictureWindow.h"
#include "vm/Interpreter.h"
#include "vm/Program.h"

using compiler::SourceMember;
using compiler::SourceMemberType;
using compiler::SourceProgram;
using shared::DialogPtr;
using shared::Label;
using shared::ViewPtr;
using shared::WindowPtr;

namespace tmbasic {

class SourceMembersListBox : public shared::ListViewer {
   public:
    SourceMembersListBox(
        const TRect& bounds,
        uint16_t numCols,
        shared::ScrollBar* vScrollBar,
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

        setRange(static_cast<int16_t>(_items.size()));

        focused = 0;
        for (size_t i = 0; i < _items.size(); i++) {
            if (_items.at(i) == selectedMember) {
                focused = static_cast<int16_t>(i);
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

void ProgramWindow::updateTitle() {
    std::ostringstream s;
    if (_dirty) {
        s << kCharBullet;
    }
    if (_filePath.has_value()) {
        s << shared::getFileName(*_filePath);
    } else {
        s << "Untitled";
    }
    s << " (Program)";

    delete[] title;  // NOLINT(cppcoreguidelines-owning-memory)
    title = newStr(s.str());
}

void ProgramWindow::enableDisableCommands(bool enable) {
    if (_contentsListBox == nullptr) {
        // Program window is closing.
        return;
    }

    TCommandSet cmds;
    if (_contentsListBox->range > 0 && _contentsListBox->focused >= 0) {
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
      _filePath(std::move(filePath)),
      _sourceProgram(std::move(sourceProgram)),
      _openMember(std::move(openMember)) {
    TCommandSet ts;
    ts.enableCmd(cmSave);
    ts.enableCmd(cmSaveAs);
    ts.enableCmd(kCmdProgramAddItem);
    ts.enableCmd(kCmdProgramRun);
    ts.enableCmd(kCmdProgramCheckForErrors);
    ts.enableCmd(kCmdProgramPublish);
    ts.enableCmd(kCmdProgramContentsWindow);
    enableCommands(ts);

    ViewPtr<shared::ScrollBar> vScrollBar{ TRect{ size.x - 1, 1, size.x, size.y - 1 } };
    vScrollBar.addTo(this);

    auto contentsListBoxRect = getExtent();
    contentsListBoxRect.grow(-1, -1);
    ViewPtr<SourceMembersListBox> contentsListBox{ contentsListBoxRect,
                                                   1,
                                                   vScrollBar,
                                                   *_sourceProgram,
                                                   [this](auto* member) -> void { this->_openMember(member); },
                                                   [this]() -> void { enableDisableCommands(true); } };
    _contentsListBox = contentsListBox;
    contentsListBox->growMode = gfGrowHiX | gfGrowHiY;
    contentsListBox.addTo(this);

    updateTitle();
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

bool ProgramWindow::save(const std::string& filePath) {
    try {
        _sourceProgram->save(filePath);
        _filePath = filePath;
        _dirty = false;
        updateTitle();
        frame->drawView();
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

bool ProgramWindow::saveAs() {
    auto d = DialogPtr<TFileDialog>("*.bas", "Save Program As (.BAS)", "~N~ame", fdOKButton, 101);
    auto result = false;

    if (TProgram::deskTop->execView(d) != cmCancel) {
        auto fileName = std::array<char, MAXPATH>();
        d->getFileName(fileName.data());
        if (save(fileName.data())) {
            result = true;
        }
    }

    return result;
}

bool ProgramWindow::save() {
    if (_filePath.has_value()) {
        return save(*_filePath);
    }
    return saveAs();
}

void ProgramWindow::deleteItem() {
    auto* member = _contentsListBox->getSelectedMember();
    if (member == nullptr) {
        return;
    }

    auto choice = messageBox(
        fmt::format("Are you sure you want to delete \"{}\"?", shared::ellipsis(member->identifier, 25)),
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

    for (auto iter = _sourceProgram->members.begin(); iter != _sourceProgram->members.end(); ++iter) {
        if (member == iter->get()) {
            _sourceProgram->members.erase(iter);
            break;
        }
    }

    _dirty = true;
    updateTitle();
    frame->drawView();
    updateListItems();
}

void ProgramWindow::handleEvent(TEvent& event) {
    if (event.what == evBroadcast) {
        switch (event.message.command) {
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
        deleteItem();
        clearEvent(event);
    } else if (event.what == evKeyDown && event.keyDown.keyCode == kbEnter) {
        auto* member = _contentsListBox->getSelectedMember();
        if (member != nullptr) {
            _openMember(member);
        }
    }

    TWindow::handleEvent(event);
}

// true = close, false = stay open
bool ProgramWindow::preClose() {
    if (_dirty) {
        std::ostringstream s;
        s << "Save changes to \"" << (_filePath.has_value() ? shared::getFileName(*_filePath) : "Untitled") << "\"?";
        auto result = messageBox(s.str(), mfWarning | mfYesNoCancel);
        if (result == cmCancel) {
            return false;
        }
        if (result == cmYes) {
            if (!save()) {
                return false;
            }
        }
    }

    return true;
}

void ProgramWindow::close() {
    closeMaybe();
}

bool ProgramWindow::closeMaybe() {
    if (preClose()) {
        // close all other program-related windows first
        message(owner, evBroadcast, kCmdCloseProgramRelatedWindows, nullptr);

        TCommandSet ts;
        ts.enableCmd(cmSave);
        ts.enableCmd(cmSaveAs);
        ts.enableCmd(kCmdProgramAddItem);
        ts.enableCmd(kCmdProgramRun);
        ts.enableCmd(kCmdProgramCheckForErrors);
        ts.enableCmd(kCmdProgramPublish);
        ts.enableCmd(kCmdProgramContentsWindow);
        disableCommands(ts);

        // Don't try to mess around with the controls during window shutdown.
        _contentsListBox = nullptr;

        TWindow::close();
        return true;
    }

    return false;
}

bool ProgramWindow::isDirty() const {
    return _dirty;
}

void ProgramWindow::setDirty() {
    _dirty = true;
    updateTitle();
    frame->drawView();
}

void ProgramWindow::addNewSourceMember(std::unique_ptr<SourceMember> sourceMember) {
    _sourceProgram->members.push_back(std::move(sourceMember));
    _contentsListBox->updateItems();
    _dirty = true;
    updateTitle();
}

void ProgramWindow::updateListItems() {
    _contentsListBox->updateItems();
}

void ProgramWindow::redrawListItems() {
    _contentsListBox->drawView();
}

void ProgramWindow::setState(uint16_t aState, bool enable) {
    TWindow::setState(aState, enable);

    if (aState == sfActive) {
        enableDisableCommands(enable);
    }
}

static void compilerErrorMessageBox(const compiler::CompilerException& ex) {
    // Some extra newlines at the end seem to be necessary to avoid the last line getting cut off.
    messageBox(
        fmt::format(
            "Error in \"{}\"\nLn {}, Col {}\n{}\n\n\n", ex.token.sourceMember->identifier, ex.token.lineIndex + 1,
            ex.token.columnIndex + 1, ex.message),
        mfError | mfOKButton);
}

void ProgramWindow::checkForErrors() {
    compiler::CompiledProgram program;
    try {
        compiler::compileProgram(*_sourceProgram, &program);
        messageBox("No errors!", mfInformation | mfOKButton);
    } catch (compiler::CompilerException& ex) {
        compilerErrorMessageBox(ex);
    }
}

class PublishStatusWindow : public TWindow {
   public:
    ViewPtr<Label> labelTop{ TRect{ 0, 0, 30, 1 }, "Checking for errors..." };
    ViewPtr<Label> labelBottom{ TRect{ 0, 0, 30, 1 }, "" };
    PublishStatusWindow() : TWindow(TRect{ 0, 0, 0, 0 }, "Please Wait", wnNoNumber), TWindowInit(initFrame) {
        palette = wpGrayWindow;
        options |= ofCentered;
        flags &= ~(wfClose | wfZoom | wfGrow | wfMove);
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
    if (!_filePath.has_value()) {
        messageBox("Please save your program first.", mfError | mfOKButton);
        return;
    }

    WindowPtr<PublishStatusWindow> const statusWindow{};
    statusWindow.get()->drawView();
    TScreen::flushScreen();

    compiler::CompiledProgram program;
    try {
        compiler::compileProgram(*_sourceProgram, &program);
    } catch (compiler::CompilerException& ex) {
        statusWindow.get()->close();
        compilerErrorMessageBox(ex);
        return;
    }

    try {
        compiler::Publisher publisher{ program, *_filePath };

        for (auto platform : compiler::getTargetPlatforms()) {
            statusWindow.get()->labelTop->setTitle("Publishing:");
            statusWindow.get()->labelTop->drawView();
            statusWindow.get()->labelBottom->setTitle(compiler::getPlatformName(platform));
            statusWindow.get()->labelBottom->drawView();
            statusWindow.get()->drawView();
            TScreen::flushScreen();
            publisher.publish(platform);
        }
        statusWindow.get()->close();
        messageBox("Publish successful!", mfInformation | mfOKButton);
    } catch (const std::runtime_error& ex) {
        statusWindow.get()->close();
        messageBox(ex.what(), mfError | mfOKButton);
    }
}

void ProgramWindow::run() {
    compiler::CompiledProgram program;
    try {
        compiler::compileProgram(*_sourceProgram, &program);
    } catch (compiler::CompilerException& ex) {
        compilerErrorMessageBox(ex);
        return;
    }

    TProgram::application->suspend();
    program.run();

    std::cout << "\nPress Enter to return to TMBASIC." << '\n';
    std::cin.get();

    TProgram::application->resume();
    TProgram::application->redraw();
}

}  // namespace tmbasic
