#include "DesignerFormPropertiesDialog.h"
#include "Button.h"
#include "CheckBoxes.h"
#include "RowLayout.h"
#include "InputLine.h"
#include "Label.h"
#include "GridLayout.h"
#include "constants.h"

using tui::UserForm;

namespace tmbasic {

enum CheckboxIndex { kShowCloseButtonIndex, kShowMaximizeButtonIndex, kAllowResizeIndex };

DesignerFormPropertiesDialog::DesignerFormPropertiesDialog(UserForm* form)
    : TDialog(TRect(0, 0, 0, 0), "Form Properties"),
      TWindowInit(&TDialog::initFrame),
      _form(form),
      _nameText(new InputLine(form->name)),
      _titleText(new InputLine(form->title)),
      _checkBoxes(new CheckBoxes(
          { "Show ~c~lose button in titlebar", "Show ~m~aximize button in titlebar", "Allow user to ~r~esize" },
          { form->showCloseButton, form->showMaximizeButton, form->allowResize })) {
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
        _form->name = _nameText->data;
        _form->title = _titleText->data;
        _form->showCloseButton = _checkBoxes->mark(kShowCloseButtonIndex);
        _form->showMaximizeButton = _checkBoxes->mark(kShowMaximizeButtonIndex);
        _form->allowResize = _checkBoxes->mark(kAllowResizeIndex);
    }

    TDialog::handleEvent(event);
}

}  // namespace tmbasic
