#pragma once

#include "../common.h"

namespace tmbasic {

class DesignerFormPropertiesDialog : public TDialog {
   public:
    DesignerFormPropertiesDialog();

   private:
    TInputLine* _nameText;
    TInputLine* _titleText;
    TCheckBoxes* _checkBoxes;
};

}  // namespace tmbasic
