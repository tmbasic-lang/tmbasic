#pragma once

#include "../common.h"
#include "tmbasic/DesignerFormProperties.h"

namespace tmbasic {

class DesignerFormPropertiesDialog : public TDialog {
   public:
    explicit DesignerFormPropertiesDialog(DesignerFormProperties* props);
    void handleEvent(TEvent& event) override;

   private:
    DesignerFormProperties* _props;
    TInputLine* _nameText;
    TInputLine* _titleText;
    TCheckBoxes* _checkBoxes;
};

}  // namespace tmbasic
