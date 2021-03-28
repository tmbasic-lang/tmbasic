#pragma once

#include "../common.h"
#include "Button.h"

// derived from tvision's TButton class

/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

namespace util {

class ThinButton : public Button {
   public:
    ThinButton(const TRect& bounds, TStringView aTitle, uint16_t aCommand, uint16_t aFlags);
    ThinButton(TStringView aTitle, ushort aCommand, ushort aFlags);

    void draw() override;
    void drawState(bool down);
    void handleEvent(TEvent& event) override;

   private:
    void drawTitle(TDrawBuffer& b, int s, int i, TAttrPair cButton, bool down);
};

}  // namespace util
