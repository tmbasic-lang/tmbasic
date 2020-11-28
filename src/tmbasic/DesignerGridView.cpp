#include "DesignerGridView.h"

namespace tmbasic {

static const char kDesignerGridViewPalette[] = "\x01\x0D";

DesignerGridView::DesignerGridView(const TRect& bounds) : TView(bounds) {}

void DesignerGridView::draw() {
    TView::draw();
    auto color = getColor(2);
    TDrawBuffer b;
    b.moveChar(0, '.', color, 1);

    for (int16_t y = 0; y < size.y; y++) {
        for (int16_t x = 1; x < size.x; x += 2) {
            writeBuf(x, y, 1, 1, b);
        }
    }
}

TPalette& DesignerGridView::getPalette() const {
    static auto palette = TPalette(kDesignerGridViewPalette, sizeof(kDesignerGridViewPalette) - 1);
    return palette;
}

}  // namespace tmbasic
