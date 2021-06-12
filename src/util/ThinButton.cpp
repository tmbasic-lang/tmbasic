#include "ThinButton.h"

// derived from tvision's TButton class

/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

namespace util {

const int cmGrabDefault = 61, cmReleaseDefault = 62;

ThinButton::ThinButton(const TRect& bounds, TStringView aTitle, uint16_t aCommand, uint16_t aFlags)
    : Button(bounds, aTitle, aCommand, aFlags) {}

ThinButton::ThinButton(TStringView aTitle, uint16_t aCommand, uint16_t aFlags)
    : Button(TRect(0, 0, cstrlen(aTitle) + 5, 1), aTitle, aCommand, aFlags) {}

void ThinButton::draw() {
    drawState(false);
}

void ThinButton::drawTitle(TDrawBuffer* b, int s, int i, TAttrPair cButton, bool down) {
    int l = 0;
    if ((flags & bfLeftJust) != 0) {
        l = 1;
    } else {
        l = (s - cstrlen(title) - 1) / 2;
        if (l < 1) {
            l = 1;
        }
    }
    b->moveCStr(i + l, title, cButton);

    if (showMarkers && !down) {
        int scOff = 4;
        if ((state & sfSelected) != 0) {
            scOff = 0;
        } else if (amDefault) {
            scOff = 2;
        }
        b->putChar(0, specialChars[scOff]);
        b->putChar(s, specialChars[scOff + 1]);
    }
}

void ThinButton::drawState(bool down) {
    TAttrPair cButton{};
    TDrawBuffer b;

    if ((state & sfDisabled) != 0) {
        cButton = getColor(0x0404);
    } else {
        cButton = getColor(0x0501);
        if ((state & sfActive) != 0) {
            if ((state & sfSelected) != 0) {
                cButton = getColor(0x0703);
            } else if (amDefault) {
                cButton = getColor(0x0602);
            }
        }
    }

    if (down) {
        cButton = { 0xA0, 0xA0 };
    }

    int s = size.x - 1;
    b.moveChar(0, ' ', cButton, size.x);

    if (title != nullptr) {
        drawTitle(&b, s, 1, cButton, down);
    }

    if (showMarkers && !down) {
        b.putChar(1, '[');
        b.putChar(s - 1, ']');
    }
    writeLine(0, 0, static_cast<int16_t>(size.x), 1, b);
}

void ThinButton::handleEvent(TEvent& event) {
    TPoint mouse{};
    TRect clickRect;

    clickRect = getExtent();

    if (event.what == evMouseDown) {
        mouse = makeLocal(event.mouse.where);
        if (!clickRect.contains(mouse)) {
            clearEvent(event);
        }
    }
    if (flags & bfGrabFocus) {
        TView::handleEvent(event);
    }

    char c = hotKey(title);
    switch (event.what) {
        case evMouseDown:
            if ((state & sfDisabled) == 0) {
                clickRect.b.x++;
                bool down = false;
                do {
                    mouse = makeLocal(event.mouse.where);
                    if (down != clickRect.contains(mouse)) {
                        down = !down;
                        drawState(down);
                    }
                } while (mouseEvent(event, evMouseMove));
                if (down) {
                    press();
                    drawState(false);
                }
            }
            clearEvent(event);
            break;

        case evKeyDown:
            if (event.keyDown.keyCode != 0 &&
                (event.keyDown.keyCode == getAltCode(c) ||
                 (owner->phase == phPostProcess && c != 0 && toupper(event.keyDown.charScan.charCode) == c) ||
                 ((state & sfFocused) != 0 && event.keyDown.charScan.charCode == ' '))) {
                press();
                clearEvent(event);
            }
            break;

        case evBroadcast:
            switch (event.message.command) {
                case cmDefault:
                    if (amDefault && !(state & sfDisabled)) {
                        press();
                        clearEvent(event);
                    }
                    break;

                case cmGrabDefault:
                case cmReleaseDefault:
                    if ((flags & bfDefault) != 0) {
                        amDefault = event.message.command == cmReleaseDefault;
                        drawView();
                    }
                    break;

                case cmCommandSetChanged:
                    setState(sfDisabled, !commandEnabled(command));
                    drawView();
                    break;
            }
            break;
    }
}

}  // namespace util
