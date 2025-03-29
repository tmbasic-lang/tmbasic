#include "AboutDialog.h"
#include "../shared/Button.h"
#include "../shared/Label.h"
#include "../shared/PictureView.h"
#include "GridLayout.h"
#include "helpfile.h"

using shared::Button;
using shared::Label;
using shared::Picture;
using shared::PictureView;

namespace tmbasic {

static const std::string kLogoPicture =
    "picture Logo Z 2e 7 N e29684 800080 c0c0c0 A A A A A A A B 20 E ff A A A A E ffffff A A A A A A A A A A A A A A A "
    "A A A A A A A A A A A A A A A A F e29680 800080 A A B e29688 A B e29680 A A B 20 E ff A A A A B e29688 A A A A B "
    "e29684 F 20 ffffff F e29684 ff B e29688 A A A B e29684 F 20 ffffff F e29684 ff B e29688 A A A B e29684 F 20 "
    "ffffff F e29688 ff A A A F 20 ffffff F e29684 ff B e29688 A A A B e29684 B 20 A A F e29688 800080 A F 20 ff F "
    "e29688 800080 A B 20 E ff E 800080 B e29688 A B 20 F e29688 ff A F 20 ffffff A F e29688 ff A F 20 ffffff F e29688 "
    "ff A F 20 ffffff A F e29688 ff A F 20 ffffff F e29688 ff A F 20 ffffff A F e29680 ff A F 20 ffffff A F e29688 ff "
    "A F 20 ffffff A F e29688 ff A F 20 ffffff A F e29680 ff A B 20 A A F e29688 800080 A B 20 B e29688 A A B e29684 B "
    "e29688 A A F 20 ff B e29688 A A A A B e29684 F 20 ffffff F e29688 ff A A A A A F 20 ffffff F e29680 ff B e29688 A "
    "A A B e29684 F 20 ffffff A F e29688 ff A F 20 ffffff A F e29688 ff A F 20 ffffff A A A E ff A A F e29688 800080 A "
    "B 20 B e29688 A B e29680 B e29688 B e29680 B e29688 A F 20 ff B e29688 A F 20 ffffff A F e29688 ff A F 20 ffffff "
    "F e29688 ff A F 20 ffffff A F e29688 ff A F 20 ffffff F e29684 ff A F 20 ffffff A F e29688 ff A F 20 ffffff A F "
    "e29688 ff A F 20 ffffff A F e29688 ff A F 20 ffffff A F e29684 ff A B 20 A A F e29680 800080 A F 20 ff F e29688 "
    "800080 A F 20 ff A A F e29688 800080 A F 20 ff B e29688 A A A A B e29680 F 20 ffffff F e29688 ff A F 20 ffffff A "
    "F e29688 ff A F 20 ffffff F e29680 ff B e29688 A A A B e29680 F 20 ffffff F e29688 ff A A A F 20 ffffff F e29680 "
    "ff B e29688 A A A B e29680 F 20 ffffff A A A E ff A F e29688 800080 A F 20 ff A A F e29688 800080 A F 20 ff E "
    "ffffff A A A A A A A A A E ff A A A A A A A A A A A A A A A A A A A A A end picture";

static Picture getLogoPicture() {
    Picture picture{ kLogoPicture };
    // replace RGB #C0C0C0 with BIOS color 7 otherwise it won't exactly match the background of the dialog box
    TColorRGB const c0c0c0{ 0xC0C0C0 };
    for (auto& cell : picture.cells) {
        if (getBack(cell.colorAttr) == c0c0c0) {
            cell.colorAttr = { getFore(cell.colorAttr), TColorBIOS{ 7 } };
        }
    }
    return picture;
}

AboutDialog::AboutDialog() : TDialog(TRect(0, 0, 0, 0), "About TMBASIC"), TWindowInit(initFrame) {
    GridLayout(
        1,
        {
            new PictureView(getLogoPicture(), 0x7),
            nullptr,
            new Label("© 2020-2024 Brian Luft"),
            new Label("tmbasic.com"),
        })
        .setRowSpacing(0)
        .setRowHeight(3, 2)
        .setRowHeight(1, 1)
        .addTo(this);

    RowLayout(true, { new Button("Close", cmOK, bfDefault) }).addTo(this, 0, size.x - 3, size.y - 3);
}

uint16_t AboutDialog::getHelpCtx() {
    return hcide_aboutDialog;
}

}  // namespace tmbasic
