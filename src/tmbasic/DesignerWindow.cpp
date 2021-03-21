#include "tmbasic/DesignerWindow.h"
#include "../../obj/resources/help/helpfile.h"
#include "../util/DialogPtr.h"
#include "../util/ViewPtr.h"
#include "../vm/UserForm.h"
#include "DesignerFormPropertiesDialog.h"
#include "DesignerGridView.h"
#include "events.h"

using compiler::SourceMember;
using util::DialogPtr;
using util::ViewPtr;
using vm::UserForm;

namespace tmbasic {

static std::string getDesignerWindowTitle(const SourceMember& member) {
    return member.identifier + " (Design)";
}

DesignerWindow::DesignerWindow(const TRect& r, SourceMember* member, std::function<void()> onEdited)
    : TWindow(r, getDesignerWindowTitle(*member), wnNoNumber),
      TWindowInit(TWindow::initFrame),
      _member(member),
      _onEdited(std::move(onEdited)) {
    options |= ofTileable;

    auto gridViewRect = getExtent();
    gridViewRect.grow(-1, -1);
    auto gridView = ViewPtr<DesignerGridView>(gridViewRect);
    gridView->growMode = gfGrowHiX | gfGrowHiY;
    gridView.addTo(this);
}

void DesignerWindow::handleEvent(TEvent& event) {
    TWindow::handleEvent(event);
    if (event.what == evCommand) {
        if (event.message.command == kCmdDesignerWindowProperties) {
            openPropertiesDialog();
            clearEvent(event);
        }
    } else if (event.what == evMouseDown) {
        if ((event.mouse.buttons & mbRightButton) != 0) {
            auto& menu = *new TMenuItem("~P~roperties", kCmdDesignerWindowProperties, kbNoKey);
            popupMenu(event.mouse.where, menu, owner);
        } else if ((event.mouse.eventFlags & meDoubleClick) != 0U) {
            openPropertiesDialog();
        }
    } else if (event.what == evBroadcast) {
        switch (event.message.command) {
            case kCmdCloseProgramRelatedWindows:
                close();
                break;

            case kCmdFindDesignerWindow: {
                auto* e = static_cast<FindDesignerWindowEventArgs*>(event.message.infoPtr);
                if (e->member == _member) {
                    e->window = this;
                    clearEvent(event);
                }
                break;
            }

            case kCmdTimerTick:
                onTimerTick();
                break;
        }
    }
}

uint16_t DesignerWindow::getHelpCtx() {
    return hcide_designerWindow;
}

void DesignerWindow::close() {
    TWindow::close();
}

void DesignerWindow::onTimerTick() {}

TPalette& DesignerWindow::getPalette() const {
    static auto palette = TPalette(cpGrayDialog, sizeof(cpGrayDialog) - 1);
    return palette;
}

void DesignerWindow::openPropertiesDialog() {
    UserForm f;
    auto dialog = DialogPtr<DesignerFormPropertiesDialog>(&f);
    dialog->options |= ofCentered;
    owner->execView(dialog);
}

void DesignerWindow::setState(uint16_t aState, bool enable) {
    TWindow::setState(aState, enable);

    if (aState == sfActive) {
        TCommandSet ts;
        ts.enableCmd(kCmdDesignAddButton);
        ts.enableCmd(kCmdDesignAddCheckBox);
        ts.enableCmd(kCmdDesignAddGroupBox);
        ts.enableCmd(kCmdDesignAddLabel);
        ts.enableCmd(kCmdDesignAddListBox);
        ts.enableCmd(kCmdDesignAddRadioButton);
        ts.enableCmd(kCmdDesignAddScrollBar);
        ts.enableCmd(kCmdDesignAddTextBox);
        ts.enableCmd(kCmdDesignAddCustomControl);
        (enable ? enableCommands : disableCommands)(ts);
    }
}

}  // namespace tmbasic
