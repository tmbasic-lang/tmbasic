#include "GridLayout.h"

namespace tmbasic {

GridLayout::GridLayout() {}

GridLayout::GridLayout(int numColumns, std::initializer_list<std::variant<TView*, RowLayout>> items) {
    int rowIndex = 0;
    int columnIndex = 0;
    for (auto& item : items) {
        // skip null views, used to leave a cell blank
        if (!std::holds_alternative<TView*>(item) || std::get<TView*>(item)) {
            addVariant(rowIndex, columnIndex, item);
        }

        columnIndex++;
        if (columnIndex == numColumns) {
            columnIndex = 0;
            rowIndex++;
        }
    }
}

void GridLayout::setMarginX(int margin) {
    _marginX = margin;
}

void GridLayout::setMarginY(int margin) {
    _marginY = margin;
}

static void resizeIfNeeded(std::vector<int>* vec, int index) {
    assert(index >= 0);
    while (static_cast<size_t>(index) >= vec->size()) {
        vec->push_back(0);
    }
}

void GridLayout::setRowHeight(int rowIndex, int height) {
    resizeIfNeeded(&_rowHeights, rowIndex);
    _rowHeights[rowIndex] = height;
}

void GridLayout::setColumnWidth(int columnIndex, int width) {
    resizeIfNeeded(&_columnWidths, columnIndex);
    _columnWidths[columnIndex] = width;
}

void GridLayout::setRowSpacing(int spacing) {
    _rowSpacing = spacing;
}

void GridLayout::setColumnSpacing(int spacing) {
    _columnSpacing = spacing;
}

void GridLayout::add(int rowIndex, int columnIndex, TView* view) {
    addVariant(rowIndex, columnIndex, view);
}

void GridLayout::add(int rowIndex, int columnIndex, const RowLayout& flow) {
    addVariant(rowIndex, columnIndex, flow);
}

void GridLayout::addVariant(int rowIndex, int columnIndex, std::variant<TView*, RowLayout> item) {
    resizeIfNeeded(&_rowHeights, rowIndex);
    resizeIfNeeded(&_columnWidths, columnIndex);
    _views.push_back({ rowIndex, columnIndex, item });
}

GridLayout::TableView GridLayout::getCell(int rowIndex, int columnIndex) {
    for (auto& v : _views) {
        if (v.rowIndex == rowIndex && v.columnIndex == columnIndex) {
            return v;
        }
    }
    return { rowIndex, columnIndex, nullptr };
}

void GridLayout::calculateRowHeights(int finalRowHeights[]) {
    for (size_t rowIndex = 0; rowIndex < _rowHeights.size(); rowIndex++) {
        auto isLastRow = rowIndex == _rowHeights.size() - 1;
        auto height = _rowHeights[rowIndex];
        if (height == 0) {
            // auto size based on contents
            auto maxContentHeight = 0;
            for (size_t columnIndex = 0; columnIndex < _columnWidths.size(); columnIndex++) {
                auto tableView = getCell(rowIndex, columnIndex);
                if (std::holds_alternative<TView*>(tableView.item)) {
                    auto* view = std::get<TView*>(tableView.item);
                    if (view) {
                        maxContentHeight = max(maxContentHeight, view->size.y);
                    }
                } else {
                    auto& flow = std::get<RowLayout>(tableView.item);
                    maxContentHeight = max(maxContentHeight, flow.getSize().y);
                }
            }
            if (!isLastRow) {
                maxContentHeight += _rowSpacing;
            }
            finalRowHeights[rowIndex] = maxContentHeight;
        } else {
            // fixed size
            if (!isLastRow) {
                height += _rowSpacing;
            }
            finalRowHeights[rowIndex] = height;
        }
    }
}

void GridLayout::calculateColumnWidths(int finalColumnWidths[]) {
    for (size_t columnIndex = 0; columnIndex < _columnWidths.size(); columnIndex++) {
        auto isLastColumn = columnIndex == _columnWidths.size() - 1;
        auto width = _columnWidths[columnIndex];
        if (width == 0) {
            // auto size based on contents
            auto maxContentWidth = 0;
            for (size_t rowIndex = 0; rowIndex < _rowHeights.size(); rowIndex++) {
                auto tableView = getCell(rowIndex, columnIndex);
                if (std::holds_alternative<TView*>(tableView.item)) {
                    auto* view = std::get<TView*>(tableView.item);
                    if (view) {
                        maxContentWidth = max(maxContentWidth, view->size.x);
                    }
                } else {
                    auto& flow = std::get<RowLayout>(tableView.item);
                    maxContentWidth = max(maxContentWidth, flow.getSize().x);
                }
            }
            if (!isLastColumn) {
                maxContentWidth += _columnSpacing;
            }
            finalColumnWidths[columnIndex] = maxContentWidth;
        } else {
            // fixed size
            if (!isLastColumn) {
                width += _columnSpacing;
            }
            finalColumnWidths[columnIndex] = width;
        }
    }
}

// returns the x,y coordinate of the bottom-right of the located views
TPoint GridLayout::apply(TGroup* group, TPoint upperLeft) {
    auto numRows = _rowHeights.size();
    auto numColumns = _columnWidths.size();

    int rowHeights[numRows] = {};
    calculateRowHeights(rowHeights);

    int columnWidths[numColumns] = {};
    calculateColumnWidths(columnWidths);

    int right = 0;
    int bottom = 0;

    auto y = upperLeft.y + _marginY;
    for (size_t rowIndex = 0; rowIndex < numRows; rowIndex++) {
        auto x = upperLeft.x + _marginX;
        auto height = rowHeights[rowIndex];

        for (size_t columnIndex = 0; columnIndex < numColumns; columnIndex++) {
            auto width = columnWidths[columnIndex];
            auto tableView = getCell(rowIndex, columnIndex);
            if (std::holds_alternative<TView*>(tableView.item)) {
                auto* view = std::get<TView*>(tableView.item);
                if (view) {
                    auto bounds = TRect(x, y, x + width - 1, y + view->size.y);
                    view->locate(bounds);
                    group->insert(view);
                }
            } else {
                auto& flow = std::get<RowLayout>(tableView.item);
                flow.addTo(group, x, x + width - 1, y);
            }

            x += width;
            right = max(right, x - 1);
        }

        y += height;
        bottom = max(bottom, y - 1);
    }

    return { right, bottom };
}

void GridLayout::addTo(TWindow* window) {
    // only the first column of views appears if the initial size of the window is 0,0. a better fix than this is
    // desirable.
    TRect dummy(0, 0, 1000, 1000);
    window->locate(dummy);

    auto bottomRight = apply(window, TPoint{ 0, 0 });
    bottomRight.x += _marginX;
    bottomRight.y += _marginY;
    TRect r(0, 0, bottomRight.x, bottomRight.y);
    window->locate(r);
}

}  // namespace tmbasic
