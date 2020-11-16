#include "tmbasic/tvutil.h"

namespace tmbasic {

void enableDisableCommand(ushort command, bool enable) {
    TCommandSet s;
    s += command;
    if (enable) {
        TView::enableCommands(s);
    } else {
        TView::disableCommands(s);
    }
}

}  // namespace tmbasic
