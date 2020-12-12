#include "Label.h"
#include "tmbasic/tvutil.h"

namespace tmbasic {

Label::Label(TStringView text, TView* link) : TLabel(TRect(0, 0, lengthWithoutTildes(text) + 1, 1), text, link) {}

// this is the same as TLabel::draw() except we don't indent the text. we don't want any extra padding on the left.
void Label::draw() {
    ushort color;
    TDrawBuffer b;
    uchar scOff;

    if (light) {
        color = getColor(0x0402);
        scOff = 0;
    } else {
        color = getColor(0x0301);
        scOff = 4;
    }

    b.moveChar(0, ' ', color, size.x);
    if (text != 0) {
        b.moveCStr(0, text, color);
    }
    if (showMarkers) {
        b.putChar(0, specialChars[scOff]);
    }
    writeLine(0, 0, size.x, 1, b);
}

}  // namespace tmbasic
