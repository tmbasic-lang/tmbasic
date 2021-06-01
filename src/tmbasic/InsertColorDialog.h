#pragma once

#include "../common.h"

namespace tmbasic {

class InsertColorDialog : public TDialog {
   public:
    static bool go(const std::string& title, const std::string& insertButtonText, TColorRGB* output);
    InsertColorDialog(const std::string& title, const std::string& insertButtonText);
    virtual ~InsertColorDialog();
    void handleEvent(TEvent& event) override;
    TColorRGB selection{};
    bool didClickHelp = false;

   private:
    gsl::owner<class InsertColorDialogPrivate*> _private;
};

}  // namespace tmbasic
