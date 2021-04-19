#include "ProgramWindow.h"
#include "../../obj/resources/help/helpfile.h"
#include "../util/DialogPtr.h"
#include "../util/Frame.h"
#include "../util/ScrollBar.h"
#include "../util/ViewPtr.h"
#include "../util/path.h"
#include "../vm/Program.h"
#include "SourceMemberTypesListBox.h"
#include "constants.h"
#include "events.h"
#include "../util/tvutil.h"

using compiler::SourceMember;
using compiler::SourceProgram;
using util::DialogPtr;
using util::ViewPtr;
using vm::Program;

namespace tmbasic {

static gsl::owner<TFrame*> initProgramWindowFrame(TRect r) {
    ViewPtr<util::Frame> f{ r };
    f->colorPassiveFrame = { 0x87 };
    f->colorPassiveTitle = { 0x87 };
    f->colorActiveFrame = { 0x8F };
    f->colorActiveTitle = { 0x8F };
    f->colorIcons = { 0x8A };
    return f.take();
}

ProgramWindow::ProgramWindow(
    const TRect& r,
    std::unique_ptr<SourceProgram> sourceProgram,
    std::optional<std::string> filePath,
    std::function<void(SourceMember*)> openMember)
    : TWindow(r, "Untitled - Program", wnNoNumber),
      TWindowInit(initProgramWindowFrame),
      _vmProgram(std::make_unique<Program>()),
      _sourceProgram(std::move(sourceProgram)),
      _dirty(false),
      _openMember(std::move(openMember)) {
    TCommandSet ts;
    ts.enableCmd(cmSave);
    ts.enableCmd(cmSaveAs);
    enableCommands(ts);

    auto vScrollBar = ViewPtr<util::ScrollBar>(TRect(size.x - 1, 1, size.x, size.y - 1));
    vScrollBar->useWhiteColorScheme();
    vScrollBar.addTo(this);

    auto typesListBoxRect = getExtent();
    typesListBoxRect.grow(-1, -1);
    typesListBoxRect.b.x = 15;
    auto typesListBox = ViewPtr<SourceMemberTypesListBox>(
        typesListBoxRect, 1, [this]() { _contentsListBox->selectType(_typesListBox->getSelectedType()); });
    _typesListBox = typesListBox;
    _typesListBox->growMode = gfGrowHiY;
    _typesListBox->options |= ofFramed;
    _typesListBox->useDarkGrayPalette();
    typesListBox.addTo(this);

    auto contentsListBoxRect = getExtent();
    contentsListBoxRect.grow(-1, -1);
    contentsListBoxRect.a.x = 16;
    auto contentsListBox = ViewPtr<SourceMembersListBox>(
        contentsListBoxRect, 1, vScrollBar, *_sourceProgram, [this](auto* member) -> void { _openMember(member); });
    _contentsListBox = contentsListBox;
    _contentsListBox->growMode = gfGrowHiX | gfGrowHiY;
    _contentsListBox->useDarkGrayPalette();
    contentsListBox.addTo(this);

    _filePath = std::move(filePath);
    updateTitle();
}

ProgramWindow::~ProgramWindow() = default;

TPalette& ProgramWindow::getPalette() const {
    static auto palette = TPalette(cpBlueDialog, sizeof(cpBlueDialog) - 1);
    return palette;
}

uint16_t ProgramWindow::getHelpCtx() {
    return hcide_programWindow;
}

void ProgramWindow::handleEvent(TEvent& event) {
    TWindow::handleEvent(event);
    if (event.what == evBroadcast) {
        switch (event.message.command) {
            case kCmdProgramSave:
                onSave();
                break;

            case kCmdProgramSaveAs:
                onSaveAs();
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
    }
}

bool ProgramWindow::onSave() {
    if (_filePath.has_value()) {
        return save(*_filePath);
    }
    return onSaveAs();
}

bool ProgramWindow::onSaveAs() {
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

void ProgramWindow::updateTitle() {
    std::ostringstream s;
    if (_dirty) {
        s << kCharBullet;
    }
    if (_filePath.has_value()) {
        s << util::getFileName(*_filePath);
    } else {
        s << "Untitled";
    }
    s << " (Program)";
    
    delete[] title;
    title = newStr(s.str().c_str());
}

// true = close, false = stay open
bool ProgramWindow::preClose() {
    if (_dirty) {
        std::ostringstream s;
        s << "Save changes to \"" << (_filePath.has_value() ? util::getFileName(*_filePath) : "Untitled") << "\"?";
        auto result = messageBox(s.str(), mfWarning | mfYesNoCancel);
        if (result == cmCancel) {
            return false;
        }
        if (result == cmYes) {
            if (!onSave()) {
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
    return _dirty;
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

}  // namespace tmbasic
