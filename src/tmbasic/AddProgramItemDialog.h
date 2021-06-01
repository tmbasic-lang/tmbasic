#pragma once

#include "../common.h"

namespace tmbasic {

class AddProgramItemDialog : public TDialog {
   public:
    AddProgramItemDialog();
    ~AddProgramItemDialog();
    void handleEvent(TEvent& event) override;

   private:
    gsl::owner<class AddProgramItemDialogPrivate*> _private;
};

}  // namespace tmbasic
