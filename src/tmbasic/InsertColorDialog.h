#pragma once

#include "../common.h"

namespace tmbasic {

class InsertColorDialogPrivate;

class InsertColorDialog : public TDialog {
   public:
    InsertColorDialog(const std::string& title, const std::string& insertButtonText);
    virtual ~InsertColorDialog();
    void handleEvent(TEvent& event) override;
    TColorRGB selection{};

   private:
    gsl::owner<InsertColorDialogPrivate*> _private;
};

}  // namespace tmbasic
