#pragma once

#include "../common.h"
#include "tmbasic/RowLayout.h"

namespace tmbasic {

// std::variant is supported on macOS 10.13 but std::get is not
class ViewOrRowLayout {
   public:
    TView* view = nullptr;
    std::optional<RowLayout> rowLayout;
    ViewOrRowLayout(TView* view);          // NOLINT(runtime/explicit)
    ViewOrRowLayout(RowLayout rowLayout);  // NOLINT(runtime/explicit)
};

class GridLayout {
   public:
    GridLayout();
    GridLayout(int numColumns, std::initializer_list<ViewOrRowLayout> items);

    // setup
    GridLayout& setMarginX(int margin);
    GridLayout& setMarginY(int margin);
    GridLayout& setRowHeight(int rowIndex, int height);
    GridLayout& setColumnWidth(int columnIndex, int width);
    GridLayout& setRowSpacing(int spacing);
    GridLayout& setColumnSpacing(int spacing);
    GridLayout& add(int rowIndex, int columnIndex, TView* view);
    GridLayout& add(int rowIndex, int columnIndex, const RowLayout& flow);

    // run
    void addTo(TWindow* window);
    TPoint apply(TGroup* group, TPoint upperLeft);

   private:
    struct TableView {
        int rowIndex = 0;
        int columnIndex = 0;
        ViewOrRowLayout item;
    };

    void addVariant(int rowIndex, int columnIndex, ViewOrRowLayout item);
    TableView getCell(int rowIndex, int columnIndex);
    void calculateRowHeights(std::vector<int>* finalRowHeights);
    void calculateColumnWidths(std::vector<int>* finalColumnWidths);

    int _marginX = 2;
    int _marginY = 2;
    std::vector<int> _rowHeights;
    std::vector<int> _columnWidths;
    int _rowSpacing = 1;
    int _columnSpacing = 1;
    std::vector<TableView> _views;
};

}  // namespace tmbasic
