#include "tmbasic/tvutil.h"

namespace tmbasic {

void enableDisableCommand(uint16_t command, bool enable) {
    TCommandSet s;
    s += command;
    if (enable) {
        TView::enableCommands(s);
    } else {
        TView::disableCommands(s);
    }
}

size_t lengthWithoutTildes(TStringView text) {
    size_t len = 0;
    for (auto ch : text) {
        if (ch != '~') {
            len++;
        }
    }
    return len;
}

}  // namespace tmbasic
