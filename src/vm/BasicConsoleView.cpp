#include "BasicConsoleView.h"

namespace vm {

BasicConsoleView::BasicConsoleView(const TRect& bounds) : TView(bounds) {
    growMode = gfGrowHiX | gfGrowHiY;
}

void BasicConsoleView::draw() {
    TDrawBuffer drawBuffer;
    drawBuffer.moveChar(0, ' ', 0, size.x);

    int16_t y = 0;
    for (; y < size.y; y++) {
        writeLine(0, y, static_cast<int16_t>(size.x), 1, drawBuffer);
    }

    y = 0;
    for (const auto& row : cells) {
        writeLine(0, y, static_cast<int16_t>(row.size()), 1, row.data());
        y++;
    }
}

std::vector<TScreenCell>* BasicConsoleView::getRow(int16_t y) {
    // If the row doesn't exist, create it.
    if (y >= 0 && static_cast<size_t>(y) >= cells.size()) {
        cells.resize(y + 1);
    }

    return &cells.at(y);
}

TScreenCell* BasicConsoleView::getCell(int16_t x, int16_t y) {
    auto* row = getRow(y);

    // If the cell doesn't exist, create it.
    if (x >= 0 && static_cast<size_t>(x) >= row->size()) {
        row->resize(x + 1);
    }

    return &row->at(x);
}

void BasicConsoleView::print(const std::string& str) {
    const auto bounds = getBounds();

    std::string_view view{ str };
    size_t iPrev = 0;
    size_t i = 0;
    size_t width = 0;

    while (TText::next(view, i, width)) {
        auto currentGraphemeCluster = str.substr(iPrev, i - iPrev);

        if (currentGraphemeCluster == "\n") {
            // Carriage return + line feed.
            currentX = 0;
            currentY++;
        } else if (currentGraphemeCluster == "\r") {
            // Carriage return.
            currentX = 0;
        } else {
            // Print one cell.
            auto* cell = getCell(currentX, currentY);
            cell->attr = currentColorAttr;
            auto size = sizeof(cell->_ch._text);
            memset(&cell->_ch._text, 0, size);
            size = std::min(size, currentGraphemeCluster.size());
            memcpy(&cell->_ch._text, currentGraphemeCluster.data(), size);

            // Move to the next cell, wrapping if necessary.
            currentX++;
            if (currentX >= bounds.b.x) {
                currentX = 0;
                currentY++;
            }
        }

        iPrev = i;
    }
}

}  // namespace vm
