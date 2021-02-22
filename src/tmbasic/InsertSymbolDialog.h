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
    InsertSymbolDialog();
    void handleEvent(TEvent& event) override;
    const char* selection = nullptr;
};

}  // namespace tmbasic
