#pragma once

#include "../common.h"

namespace tmbasic {

class InsertColorDialogPrivate;

class InsertColorDialog : public TDialog {
   public:
    InsertColorDialog();
    virtual ~InsertColorDialog();
    void handleEvent(TEvent& event) override;
    TColorDesired selection{};

   private:
    gsl::owner<InsertColorDialogPrivate*> _private;
};

}  // namespace tmbasic
