#pragma once

#include "../common.h"

namespace tmbasic {

class InsertColorDialogPrivate;

class InsertColorDialog : public TDialog {
   public:
    static bool go(const std::string& title, const std::string& insertButtonText, bool foreground, TColorRGB* output);
    InsertColorDialog(const std::string& title, const std::string& insertButtonText, bool foreground);
    virtual ~InsertColorDialog();
    void handleEvent(TEvent& event) override;
    TColorRGB selection{};
    bool didClickHelp = false;

   private:
    gsl::owner<InsertColorDialogPrivate*> _private;
};

}  // namespace tmbasic
