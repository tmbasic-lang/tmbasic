#pragma once

#include "../common.h"
#include "shared/tui/UserForm.h"

namespace tmbasic {

class DesignerFormPropertiesDialog : public TDialog {
   public:
    explicit DesignerFormPropertiesDialog(tui::UserForm* form);
    void handleEvent(TEvent& event) override;

   private:
    tui::UserForm* _form;
    TInputLine* _nameText;
    TInputLine* _titleText;
    TCheckBoxes* _checkBoxes;
};

}  // namespace tmbasic
