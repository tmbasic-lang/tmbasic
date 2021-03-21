#include "DesignerFormPropertiesDialog.h"
#include "../util/Button.h"
#include "../util/InputLine.h"
#include "../util/Label.h"
#include "../util/tvutil.h"
#include "../vm/UserForm.h"
#include "GridLayout.h"
#include "RowLayout.h"
#include "constants.h"

using util::Button;
using util::InputLine;
using util::Label;
using util::parseUserInt;
using vm::UserForm;

namespace tmbasic {

enum CheckboxIndex { kShowCloseButtonIndex, kShowMaximizeButtonIndex, kAllowResizeIndex };

DesignerFormPropertiesDialog::DesignerFormPropertiesDialog(UserForm* form)
    : TDialog(TRect(0, 0, 0, 0), "Form Properties"),
      TWindowInit(&TDialog::initFrame),
      _form(form),
      _nameText(new InputLine(form->name)),
      _titleText(new InputLine(form->title)),
      _minWidthText(new InputLine(form->minWidth, 6, 4)),
      _minHeightText(new InputLine(form->minHeight, 6, 4)),
      _initialWidthText(new InputLine(form->initialWidth, 6, 4)),
      _initialHeightText(new InputLine(form->initialHeight, 6, 4)),
      _maxWidthText(new InputLine(form->maxWidth, 6, 4)),
      _maxHeightText(new InputLine(form->maxHeight, 6, 4)),
      _frameCheckBoxes(new CheckBoxes(
          { "Show ~c~lose button in titlebar", "Show ~m~aximize button in titlebar", "Allow user to ~r~esize" },
          { form->showCloseButton, form->showMaximizeButton, form->allowResize })) {
    GridLayout(
        2,
        {
            new Label("~N~ame:", _nameText),
            _nameText,
            new Label("~T~itle:", _titleText),
            _titleText,
            new Label("Frame:", _frameCheckBoxes),
            _frameCheckBoxes,
            new Label("~M~inimum size:", _minWidthText),
            RowLayout(false, { _minWidthText, new Label("x"), _minHeightText }),
            new Label("Initial ~s~ize:", _initialWidthText),
            RowLayout(false, { _initialWidthText, new Label("x"), _initialHeightText }),
            new Label("Ma~x~imum size:", _maxWidthText),
            RowLayout(false, { _maxWidthText, new Label("x"), _maxHeightText }),
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
        try {
            // these can fail so do them first and don't update the structure until after they are confirmed valid
            auto minWidth = parseUserInt(_minWidthText->data, "minimum width", 0, 1000);
            auto minHeight = parseUserInt(_minHeightText->data, "minimum height", 0, 1000);
            auto initialWidth = parseUserInt(_initialWidthText->data, "initial width", 1, 1000);
            auto initialHeight = parseUserInt(_initialHeightText->data, "initial height", 1, 1000);
            auto maxWidth = parseUserInt(_maxWidthText->data, "maximum width", 0, 1000);
            auto maxHeight = parseUserInt(_maxHeightText->data, "maximum height", 0, 1000);

            _form->name = _nameText->data;
            _form->title = _titleText->data;
            _form->showCloseButton = _frameCheckBoxes->mark(kShowCloseButtonIndex);
            _form->showMaximizeButton = _frameCheckBoxes->mark(kShowMaximizeButtonIndex);
            _form->allowResize = _frameCheckBoxes->mark(kAllowResizeIndex);
            _form->minWidth = minWidth;
            _form->minHeight = minHeight;
            _form->initialWidth = initialWidth;
            _form->initialHeight = initialHeight;
            _form->maxWidth = maxWidth;
            _form->maxHeight = maxHeight;
        } catch (std::runtime_error& ex) {
            messageBox(ex.what(), mfError | mfOKButton);
            clearEvent(event);
        }
    }

    TDialog::handleEvent(event);
}

}  // namespace tmbasic
