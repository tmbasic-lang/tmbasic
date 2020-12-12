#include "TableLayout.h"

namespace tmbasic {

TableLayout::TableLayout(int numRows, int numColumns)
    : _rowHeights(std::vector<int>(numRows, 0)), _columnWidths(std::vector<int>(numColumns, 0)) {}

void TableLayout::setMarginX(int margin) {
    _marginX = margin;
}

void TableLayout::setMarginY(int margin) {
    _marginY = margin;
}

void TableLayout::setFillRow(int rowIndex) {
    _rowHeights[rowIndex] = -1;
}

void TableLayout::setFillColumn(int columnIndex) {
    _columnWidths[columnIndex] = -1;
}

void TableLayout::setFixedSizeRow(int rowIndex, int height) {
    _rowHeights[rowIndex] = height;
}

void TableLayout::setFixedSizeColumn(int columnIndex, int width) {
    _columnWidths[columnIndex] = width;
}

void TableLayout::setRowSpacing(int spacing) {
    _rowSpacing = spacing;
}
void TableLayout::setColumnSpacing(int spacing) {
    _columnSpacing = spacing;
}

TView* TableLayout::add(int rowIndex, int columnIndex, TView* view) {
    _views.push_back({ rowIndex, columnIndex, view, nullptr, false });
    return view;
}

void TableLayout::add(int rowIndex, int columnIndex, FlowLayout* flow, bool leftAlign) {
    _views.push_back({ rowIndex, columnIndex, nullptr, flow, leftAlign });
}

TableLayout::TableView TableLayout::getCell(int rowIndex, int columnIndex) {
    for (auto& v : _views) {
        if (v.rowIndex == rowIndex && v.columnIndex == columnIndex) {
            return v;
        }
    }
    return { rowIndex, columnIndex, nullptr, nullptr };
}

void TableLayout::calculateRowHeights(int finalRowHeights[], const TRect& extent) {
    auto fillRowIndex = -1;
    auto totalHeight = 0;

    for (size_t rowIndex = 0; rowIndex < _rowHeights.size(); rowIndex++) {
        auto isLastRow = rowIndex == _rowHeights.size() - 1;
        auto height = _rowHeights[rowIndex];
        if (height == -1) {
            // expand to fill. update finalRowHeights and totalHeight at the end.
            fillRowIndex = rowIndex;
        } else if (height == 0) {
            // auto size based on contents
            auto maxContentHeight = 0;
            for (size_t columnIndex = 0; columnIndex < _columnWidths.size(); columnIndex++) {
                auto tableView = getCell(rowIndex, columnIndex);
                if (tableView.view) {
                    maxContentHeight = max(maxContentHeight, tableView.view->size.y);
                } else if (tableView.flow) {
                    maxContentHeight = max(maxContentHeight, tableView.flow->getHeight());
                }
            }
            if (!isLastRow) {
                maxContentHeight += _rowSpacing;
            }
            finalRowHeights[rowIndex] = maxContentHeight;
            totalHeight += maxContentHeight;
        } else {
            // fixed size
            if (!isLastRow) {
                height += _rowSpacing;
            }
            finalRowHeights[rowIndex] = height;
            totalHeight += height;
        }
    }

    if (fillRowIndex != -1) {
        finalRowHeights[fillRowIndex] = (extent.b.y - extent.a.y + 1) - 2 * _marginY - totalHeight;
    }
}

void TableLayout::calculateColumnWidths(int finalColumnWidths[], const TRect& extent) {
    auto fillColumnIndex = -1;
    auto totalWidth = 0;

    for (size_t columnIndex = 0; columnIndex < _columnWidths.size(); columnIndex++) {
        auto isLastColumn = columnIndex == _columnWidths.size() - 1;
        auto width = _columnWidths[columnIndex];
        if (width == -1) {
            // expand to fill. update finalColumnWidths and totalWidth at the end.
            fillColumnIndex = columnIndex;
        } else if (width == 0) {
            // auto size based on contents
            auto maxContentWidth = 0;
            for (size_t rowIndex = 0; rowIndex < _rowHeights.size(); rowIndex++) {
                auto tableView = getCell(rowIndex, columnIndex);
                if (tableView.view) {
                    maxContentWidth = max(maxContentWidth, tableView.view->size.x);
                } else if (tableView.flow) {
                    maxContentWidth = max(maxContentWidth, tableView.flow->getWidth());
                }
            }
            if (!isLastColumn) {
                maxContentWidth += _columnSpacing;
            }
            finalColumnWidths[columnIndex] = maxContentWidth;
            totalWidth += maxContentWidth;
        } else {
            // fixed size
            if (!isLastColumn) {
                width += _columnSpacing;
            }
            finalColumnWidths[columnIndex] = width;
            totalWidth += width;
        }
    }

    if (fillColumnIndex != -1) {
        finalColumnWidths[fillColumnIndex] = (extent.b.x - extent.a.x + 1) - 2 * _marginX - totalWidth;
    }
}

// returns the x,y coordinate of the bottom-right of the located views
TPoint TableLayout::apply(TRect extent) {
    auto numRows = _rowHeights.size();
    auto numColumns = _columnWidths.size();

    int rowHeights[numRows] = {};
    calculateRowHeights(rowHeights, extent);

    int columnWidths[numColumns] = {};
    calculateColumnWidths(columnWidths, extent);

    int right = 0;
    int bottom = 0;

    auto y = extent.a.y + _marginY;
    for (size_t rowIndex = 0; rowIndex < numRows; rowIndex++) {
        auto x = extent.a.x + _marginX;
        auto height = rowHeights[rowIndex];

        for (size_t columnIndex = 0; columnIndex < numColumns; columnIndex++) {
            auto width = columnWidths[columnIndex];
            auto tableView = getCell(rowIndex, columnIndex);
            if (tableView.view) {
                auto bounds = TRect(x, y, x + width - 1, y + tableView.view->size.y);
                tableView.view->locate(bounds);
            } else if (tableView.flow) {
                if (tableView.flowLeftAlign) {
                    tableView.flow->applyLeftAligned(x, y);
                } else {
                    tableView.flow->applyRightAligned(x + width - 1, y);
                }
            }

            x += width;
            right = max(right, x - 1);
        }

        y += height;
        bottom = max(bottom, y - 1);
    }

    return { right, bottom };
}

void TableLayout::apply(TWindow* window) {
    auto bottomRight = apply(window->getExtent());
    bottomRight.x += _marginX;
    bottomRight.y += _marginY;
    TRect r(0, 0, bottomRight.x, bottomRight.y);
    window->locate(r);
}

}  // namespace tmbasic
