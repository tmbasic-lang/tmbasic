#pragma once

#include "../common.h"
#include "CheckBoxes.h"
#include "InputLine.h"
#include "shared/tui/UserForm.h"

namespace tmbasic {

class DesignerFormPropertiesDialog : public TDialog {
   public:
    explicit DesignerFormPropertiesDialog(tui::UserForm* form);
    void handleEvent(TEvent& event) override;

   private:
    tui::UserForm* _form;
    InputLine* _nameText;
    InputLine* _titleText;
    CheckBoxes* _frameCheckBoxes;
    InputLine* _minWidthText;
    InputLine* _minHeightText;
    InputLine* _initialWidthText;
    InputLine* _initialHeightText;
    InputLine* _maxWidthText;
    InputLine* _maxHeightText;
};

}  // namespace tmbasic
