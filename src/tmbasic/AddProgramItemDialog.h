#pragma once

#include "../common.h"

namespace tmbasic {

class AddProgramItemDialogPrivate;

class AddProgramItemDialog : public TDialog {
   public:
    AddProgramItemDialog();
    ~AddProgramItemDialog();
    void handleEvent(TEvent& event) override;

   private:
    gsl::owner<AddProgramItemDialogPrivate*> _private;
};

}  // namespace tmbasic
