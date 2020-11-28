#include "tmbasic/DesignerWindow.h"
#include "../../obj/helpfile.h"
#include "tmbasic/DesignerGridView.h"
#include "tmbasic/events.h"

using compiler::SourceMember;
using compiler::SourceMemberType;

namespace tmbasic {

static std::string getDesignerWindowTitle(const SourceMember& member) {
    return member.identifier + " (Design)";
}

DesignerWindow::DesignerWindow(const TRect& r, SourceMember* member, std::function<void()> onEdited)
    : TWindow(r, getDesignerWindowTitle(*member), wnNoNumber),
      TWindowInit(TWindow::initFrame),
      _member(member),
      _onEdited(onEdited) {
    options |= ofTileable;

    auto gridViewRect = getExtent();
    gridViewRect.grow(-1, -1);
    auto* gridView = new DesignerGridView(gridViewRect);
    gridView->growMode = gfGrowHiX | gfGrowHiY;
    insert(gridView);
}

void DesignerWindow::handleEvent(TEvent& event) {
    TWindow::handleEvent(event);
    if (event.what == evBroadcast) {
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

}  // namespace tmbasic