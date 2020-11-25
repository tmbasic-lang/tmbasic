#include "tmbasic/DesignerWindow.h"
#include "../../obj/helpfile.h"
#include "tmbasic/events.h"

using compiler::SourceMember;
using compiler::SourceMemberType;

namespace tmbasic {

static std::string getDesignerWindowTitle(const SourceMember& member) {
    std::ostringstream s;
    s << member.identifier << " - "
      << "Form";
    return s.str();
}

DesignerWindow::DesignerWindow(const TRect& r, SourceMember* member, std::function<void()> onEdited)
    : TWindow(r, getDesignerWindowTitle(*member), wnNoNumber),
      TWindowInit(TWindow::initFrame),
      _member(member),
      _onEdited(onEdited) {
    options |= ofTileable;
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

}  // namespace tmbasic
