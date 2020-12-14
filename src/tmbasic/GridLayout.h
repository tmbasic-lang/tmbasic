#pragma once

#include "../common.h"
#include "tmbasic/RowLayout.h"

namespace tmbasic {

class GridLayout {
   public:
    GridLayout();
    GridLayout(int numColumns, std::initializer_list<std::variant<TView*, RowLayout>> items);

    // setup
    void setMarginX(int margin);
    void setMarginY(int margin);
    void setRowHeight(int rowIndex, int height);
    void setColumnWidth(int columnIndex, int width);
    void setRowSpacing(int spacing);
    void setColumnSpacing(int spacing);
    void add(int rowIndex, int columnIndex, TView* view);
    void add(int rowIndex, int columnIndex, const RowLayout& flow);

    // run
    void addTo(TWindow* window);

   private:
    struct TableView {
        int rowIndex;
        int columnIndex;
        std::variant<TView*, RowLayout> item;
    };

    void addVariant(int rowIndex, int columnIndex, std::variant<TView*, RowLayout> item);
    TableView getCell(int rowIndex, int columnIndex);
    void calculateRowHeights(int finalRowHeights[]);
    void calculateColumnWidths(int finalColumnWidths[]);
    TPoint apply(TGroup* group, TPoint upperLeft);

    int _marginX = 3;
    int _marginY = 2;
    std::vector<int> _rowHeights;
    std::vector<int> _columnWidths;
    int _rowSpacing = 1;
    int _columnSpacing = 1;
    std::vector<TableView> _views;
};

}  // namespace tmbasic
