#include "DesignerFormPropertiesDialog.h"
#include "Button.h"
#include "CheckBoxes.h"
#include "RowLayout.h"
#include "InputLine.h"
#include "Label.h"
#include "GridLayout.h"
#include "constants.h"

namespace tmbasic {

enum CheckboxIndex { kShowCloseButtonIndex, kShowMaximizeButtonIndex, kAllowResizeIndex };

DesignerFormPropertiesDialog::DesignerFormPropertiesDialog(DesignerFormProperties* props)
    : TDialog(TRect(0, 0, 0, 0), "Form Properties"),
      TWindowInit(&TDialog::initFrame),
      _props(props),
      _nameText(new InputLine(props->name)),
      _titleText(new InputLine(props->title)),
      _checkBoxes(new CheckBoxes(
          { "Show ~c~lose button in titlebar", "Show ~m~aximize button in titlebar", "Allow user to ~r~esize" },
          { props->showCloseButton, props->showMaximizeButton, props->allowResize })) {
    GridLayout(
        2,
        {
            new Label("~N~ame:", _nameText),
            _nameText,
            new Label("~T~itle:", _titleText),
            _titleText,
            new Label("Frame:", _checkBoxes),
            _checkBoxes,
            nullptr,
            RowLayout(
                true,
                {
                    new Button("OK", cmOK, bfDefault),
                    new Button("Cancel", cmCancel, bfNormal),
                }),
        })
        .addTo(this);

    _nameText->focus();
}

void DesignerFormPropertiesDialog::handleEvent(TEvent& event) {
    if (event.what == evCommand && event.message.command == cmOK) {
        _props->name = _nameText->data;
        _props->title = _titleText->data;
        _props->showCloseButton = _checkBoxes->mark(kShowCloseButtonIndex);
        _props->showMaximizeButton = _checkBoxes->mark(kShowMaximizeButtonIndex);
        _props->allowResize = _checkBoxes->mark(kAllowResizeIndex);
    }

    TDialog::handleEvent(event);
}

}  // namespace tmbasic
