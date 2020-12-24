#include "DesignerGridView.h"

namespace tmbasic {

static std::array<char, 2> kDesignerGridViewPalette = { 0x01, 0x0D };

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
    static auto palette = TPalette(kDesignerGridViewPalette.data(), kDesignerGridViewPalette.size());
    return palette;
}

}  // namespace tmbasic
