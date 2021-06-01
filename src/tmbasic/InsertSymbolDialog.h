#pragma once

#include "../common.h"

// derived from tvision's tvdemo example app

/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

namespace tmbasic {

class InsertSymbolDialog : public TDialog {
   public:
    InsertSymbolDialog(const std::string& title, const std::string& insertButtonText);
    virtual ~InsertSymbolDialog();
    void handleEvent(TEvent& event) override;
    std::string getSelection() const;

   private:
    gsl::owner<class InsertSymbolDialogPrivate*> _private;
};

}  // namespace tmbasic
