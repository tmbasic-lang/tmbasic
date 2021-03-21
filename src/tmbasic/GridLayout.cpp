#include "GridLayout.h"

namespace tmbasic {

ViewOrRowLayout::ViewOrRowLayout(TView* view) : view(view) {}

ViewOrRowLayout::ViewOrRowLayout(RowLayout rowLayout) : rowLayout(std::move(rowLayout)) {}

GridLayout::GridLayout() = default;

GridLayout::GridLayout(int numColumns, std::initializer_list<ViewOrRowLayout> items) {
    int rowIndex = 0;
    int columnIndex = 0;
    for (const auto& item : items) {
        // skip null views, used to leave a cell blank
        if ((item.view != nullptr) || item.rowLayout.has_value()) {
            addVariant(rowIndex, columnIndex, item);
        }

        columnIndex++;
        if (columnIndex == numColumns) {
            columnIndex = 0;
            rowIndex++;
        }
    }
}

GridLayout& GridLayout::setMarginX(int margin) {
    _marginX = margin;
    return *this;
}

GridLayout& GridLayout::setMarginY(int margin) {
    _marginY = margin;
    return *this;
}

static void resizeIfNeeded(std::vector<int>* vec, int index) {
    assert(index >= 0);
    while (static_cast<size_t>(index) >= vec->size()) {
        vec->push_back(0);
    }
}

GridLayout& GridLayout::setRowHeight(int rowIndex, int height) {
    resizeIfNeeded(&_rowHeights, rowIndex);
    _rowHeights[rowIndex] = height;
    return *this;
}

GridLayout& GridLayout::setColumnWidth(int columnIndex, int width) {
    resizeIfNeeded(&_columnWidths, columnIndex);
    _columnWidths[columnIndex] = width;
    return *this;
}

GridLayout& GridLayout::setRowSpacing(int spacing) {
    _rowSpacing = spacing;
    return *this;
}

GridLayout& GridLayout::setColumnSpacing(int spacing) {
    _columnSpacing = spacing;
    return *this;
}

GridLayout& GridLayout::add(int rowIndex, int columnIndex, TView* view) {
    addVariant(rowIndex, columnIndex, view);
    return *this;
}

GridLayout& GridLayout::add(int rowIndex, int columnIndex, const RowLayout& flow) {
    addVariant(rowIndex, columnIndex, flow);
    return *this;
}

void GridLayout::addVariant(int rowIndex, int columnIndex, ViewOrRowLayout item) {
    resizeIfNeeded(&_rowHeights, rowIndex);
    resizeIfNeeded(&_columnWidths, columnIndex);
    _views.push_back({ rowIndex, columnIndex, std::move(item) });
}

GridLayout::TableView GridLayout::getCell(int rowIndex, int columnIndex) {
    for (auto& v : _views) {
        if (v.rowIndex == rowIndex && v.columnIndex == columnIndex) {
            return v;
        }
    }
    return { rowIndex, columnIndex, nullptr };
}

void GridLayout::calculateRowHeights(std::vector<int>* finalRowHeights) {
    for (size_t rowIndex = 0; rowIndex < _rowHeights.size(); rowIndex++) {
        auto isLastRow = rowIndex == _rowHeights.size() - 1;
        auto height = _rowHeights[rowIndex];
        if (height == 0) {
            // auto size based on contents
            auto maxContentHeight = 0;
            for (size_t columnIndex = 0; columnIndex < _columnWidths.size(); columnIndex++) {
                auto tableView = getCell(rowIndex, columnIndex);
                if (tableView.item.rowLayout.has_value()) {
                    auto& flow = *tableView.item.rowLayout;
                    maxContentHeight = max(maxContentHeight, flow.getSize().y);
                } else {
                    if (tableView.item.view != nullptr) {
                        maxContentHeight = max(maxContentHeight, tableView.item.view->size.y);
                    }
                }
            }
            if (!isLastRow) {
                maxContentHeight += _rowSpacing;
            }
            (*finalRowHeights)[rowIndex] = maxContentHeight;
        } else {
            // fixed size
            if (!isLastRow) {
                height += _rowSpacing;
            }
            (*finalRowHeights)[rowIndex] = height;
        }
    }
}

void GridLayout::calculateColumnWidths(std::vector<int>* finalColumnWidths) {
    for (size_t columnIndex = 0; columnIndex < _columnWidths.size(); columnIndex++) {
        auto isLastColumn = columnIndex == _columnWidths.size() - 1;
        auto width = _columnWidths[columnIndex];
        if (width == 0) {
            // auto size based on contents
            auto maxContentWidth = 0;
            for (size_t rowIndex = 0; rowIndex < _rowHeights.size(); rowIndex++) {
                auto tableView = getCell(rowIndex, columnIndex);
                if (tableView.item.rowLayout.has_value()) {
                    auto& flow = *tableView.item.rowLayout;
                    maxContentWidth = max(maxContentWidth, flow.getSize().x);
                } else {
                    if (tableView.item.view != nullptr) {
                        maxContentWidth = max(maxContentWidth, tableView.item.view->size.x);
                    }
                }
            }
            if (!isLastColumn) {
                maxContentWidth += _columnSpacing;
            }
            (*finalColumnWidths)[columnIndex] = maxContentWidth;
        } else {
            // fixed size
            if (!isLastColumn) {
                width += _columnSpacing;
            }
            (*finalColumnWidths)[columnIndex] = width;
        }
    }
}

// returns the x,y coordinate of the bottom-right of the located views
TPoint GridLayout::apply(TGroup* group, TPoint upperLeft) {
    auto numRows = _rowHeights.size();
    auto numColumns = _columnWidths.size();

    auto rowHeights = std::vector<int>(numRows);
    calculateRowHeights(&rowHeights);

    auto columnWidths = std::vector<int>(numColumns);
    calculateColumnWidths(&columnWidths);

    int right = 0;
    int bottom = 0;

    auto y = upperLeft.y + _marginY;
    for (size_t rowIndex = 0; rowIndex < numRows; rowIndex++) {
        auto x = upperLeft.x + _marginX;
        auto height = rowHeights[rowIndex];

        for (size_t columnIndex = 0; columnIndex < numColumns; columnIndex++) {
            auto width = columnWidths[columnIndex];
            auto tableView = getCell(rowIndex, columnIndex);
            if (tableView.item.rowLayout.has_value()) {
                auto& flow = *tableView.item.rowLayout;
                flow.addTo(group, x, x + width - 1, y);
            } else if (tableView.item.view != nullptr) {
                auto bounds = TRect(x, y, x + width - 1, y + tableView.item.view->size.y);
                tableView.item.view->locate(bounds);
                group->insert(tableView.item.view);
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
    bottomRight.x += _marginX + 1;
    bottomRight.y += _marginY;
    TRect r(0, 0, bottomRight.x, bottomRight.y);
    window->locate(r);
}

}  // namespace tmbasic
