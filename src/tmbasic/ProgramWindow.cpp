#include "tmbasic/ProgramWindow.h"
#include "../../obj/helpfile.h"
#include "shared/util/path.h"
#include "shared/vm/Program.h"
#include "tmbasic/SourceMemberTypesListBox.h"
#include "tmbasic/constants.h"
#include "tmbasic/events.h"
#include "tmbasic/tvutil.h"

using vm::Program;

namespace tmbasic {

ProgramWindow::ProgramWindow(
    const TRect& r,
    std::unique_ptr<SourceProgram> sourceProgram,
    std::optional<std::string> filePath,
    std::function<void(SourceMember*)> openMember)
    : TWindow(r, "Untitled - Program", wnNoNumber),
      TWindowInit(TWindow::initFrame),
      _vmProgram(std::make_unique<Program>()),
      _sourceProgram(std::move(sourceProgram)),
      _dirty(false),
      _openMember(openMember) {
    palette = 0;
    auto* vScrollBar = new TScrollBar(TRect(size.x - 1, 1, size.x, size.y - 1));
    insert(vScrollBar);

    auto typesListBoxRect = getExtent();
    typesListBoxRect.grow(-1, -1);
    typesListBoxRect.b.x = 15;
    _typesListBox = new SourceMemberTypesListBox(
        typesListBoxRect, 1, [this]() { _contentsListBox->selectType(_typesListBox->getSelectedType()); });
    _typesListBox->growMode = gfGrowHiY;
    _typesListBox->options |= ofFramed;
    insert(_typesListBox);

    auto contentsListBoxRect = getExtent();
    contentsListBoxRect.grow(-1, -1);
    contentsListBoxRect.a.x = 16;
    _contentsListBox = new SourceMembersListBox(
        contentsListBoxRect, 1, vScrollBar, *_sourceProgram, [this](auto* member) -> void { _openMember(member); });
    _contentsListBox->growMode = gfGrowHiX | gfGrowHiY;
    insert(_contentsListBox);

    _filePath = filePath;
    updateTitle();
}

ProgramWindow::~ProgramWindow() {}

TPalette& ProgramWindow::getPalette() const {
    static auto palette = TPalette(cpBlueWindow, sizeof(cpBlueWindow) - 1);
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
    } else {
        return onSaveAs();
    }
}

bool ProgramWindow::onSaveAs() {
    auto* d = new TFileDialog("*.bas", "Save Program As (.BAS)", "~N~ame", fdOKButton, 101);
    auto result = false;

    if (TProgram::deskTop->execView(d) != cmCancel) {
        char fileName[MAXPATH];
        d->getFileName(fileName);
        if (save(fileName)) {
            _filePath = fileName;
            updateTitle();
            frame->drawView();
            result = true;
        }
    }
    destroy(d);

    return result;
}

bool ProgramWindow::save(std::string filePath) {
    try {
        _sourceProgram->save(filePath);
        _filePath = filePath;
        _dirty = false;
        updateTitle();
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
    s << " - Program";

    delete[] title;
    title = strdup(s.str().c_str());
}

// true = close, false = stay open
bool ProgramWindow::preClose() {
    if (_dirty) {
        std::ostringstream s;
        s << "Save changes to \"" << (_filePath.has_value() ? util::getFileName(*_filePath) : "Untitled") << "\"?";
        auto result = messageBox(s.str(), mfWarning | mfYesNoCancel);
        if (result == cmCancel) {
            return false;
        } else if (result == cmYes) {
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

        TWindow::close();
    }
}

void ProgramWindow::enableDisableMenuCommands() {
    enableDisableCommand(cmSave, _dirty);
    enableDisableCommand(cmSaveAs, true);
}

bool ProgramWindow::isDirty() {
    return _dirty;
}

void ProgramWindow::addNewSourceMember(std::unique_ptr<SourceMember> sourceMember) {
    _sourceProgram->members.push_back(std::move(sourceMember));
    _contentsListBox->updateItems();
    _dirty = true;
    updateTitle();
}

}  // namespace tmbasic
