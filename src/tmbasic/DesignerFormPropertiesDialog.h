#pragma once

#include "../common.h"
#include "../shared/CheckBoxes.h"
#include "../shared/InputLine.h"
#include "shared/UserForm.h"

namespace tmbasic {

class DesignerFormPropertiesDialog : public TDialog {
   public:
    explicit DesignerFormPropertiesDialog(shared::UserForm* form);
    void handleEvent(TEvent& event) override;

   private:
    shared::UserForm* _form;
    shared::InputLine* _nameText;
    shared::InputLine* _titleText;
    shared::CheckBoxes* _frameCheckBoxes;
    shared::InputLine* _minWidthText;
    shared::InputLine* _minHeightText;
    shared::InputLine* _initialWidthText;
    shared::InputLine* _initialHeightText;
    shared::InputLine* _maxWidthText;
    shared::InputLine* _maxHeightText;
};

}  // namespace tmbasic
