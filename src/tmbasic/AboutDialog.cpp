#include "AboutDialog.h"
#include "../../obj/resources/help/helpfile.h"
#include "Button.h"
#include "Label.h"
#include "GridLayout.h"

namespace tmbasic {

AboutDialog::AboutDialog() : TDialog(TRect(0, 0, 0, 0), "About TMBASIC"), TWindowInit(&TDialog::initFrame) {
    GridLayout(
        1,
        {
            new Label("TMBASIC"),
            new Label("(C) 2020 Brian Luft"),
            new Label("tmbasic.com"),
            nullptr,
            RowLayout(true, { new Button("Close", cmOK, bfDefault) }),
        })
        .setColumnWidth(0, 30)
        .setRowSpacing(0)
        .setRowHeight(3, 1)
        .addTo(this);
}

uint16_t AboutDialog::getHelpCtx() {
    return hcide_aboutDialog;
}

}  // namespace tmbasic
