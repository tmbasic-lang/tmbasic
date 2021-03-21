#pragma once

#include "../common.h"
#include "CheckBoxes.h"
#include "../util/InputLine.h"
#include "vm/UserForm.h"

namespace tmbasic {

class DesignerFormPropertiesDialog : public TDialog {
   public:
    explicit DesignerFormPropertiesDialog(vm::UserForm* form);
    void handleEvent(TEvent& event) override;

   private:
    vm::UserForm* _form;
    util::InputLine* _nameText;
    util::InputLine* _titleText;
    CheckBoxes* _frameCheckBoxes;
    util::InputLine* _minWidthText;
    util::InputLine* _minHeightText;
    util::InputLine* _initialWidthText;
    util::InputLine* _initialHeightText;
    util::InputLine* _maxWidthText;
    util::InputLine* _maxHeightText;
};

}  // namespace tmbasic
