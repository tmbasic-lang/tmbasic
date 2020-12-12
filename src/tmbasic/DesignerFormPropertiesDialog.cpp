#include "DesignerFormPropertiesDialog.h"
#include "Button.h"
#include "CheckBoxes.h"
#include "FlowLayout.h"
#include "InputLine.h"
#include "Label.h"
#include "TableLayout.h"

namespace tmbasic {

static const auto kDialogWidth = 55;
static const auto kDialogHeight = 17;
static const auto kButtonWidth = 13;

DesignerFormPropertiesDialog::DesignerFormPropertiesDialog()
    : TDialog(TRect(0, 0, kDialogWidth, kDialogHeight), "Form Properties"), TWindowInit(&TDialog::initFrame) {
    TableLayout table(4, 3);

    insert(table.add(0, 1, _nameText = new InputLine()));
    insert(table.add(0, 0, new Label("~N~ame:", _nameText)));

    insert(table.add(1, 1, _titleText = new InputLine()));
    insert(table.add(1, 0, new Label("~T~itle:", _titleText)));

    insert(table.add(
        2, 1,
        _checkBoxes = new CheckBoxes({
            "Show ~c~lose button in titlebar",
            "Show ~m~aximize button in titlebar",
            "Allow user to ~r~esize",
        })));
    insert(table.add(2, 0, new Label("Frame:", _checkBoxes)));

    FlowLayout flow;
    insert(flow.add(new Button("OK", cmOK, bfDefault)));
    insert(flow.add(new Button("Cancel", cmCancel, bfNormal)));
    table.add(3, 1, &flow, false);

    table.apply(this);

    _nameText->focus();
}

}  // namespace tmbasic
