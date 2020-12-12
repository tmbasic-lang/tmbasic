#pragma once

#include "../common.h"
#include "tmbasic/FlowLayout.h"

namespace tmbasic {

class TableLayout {
   public:
    TableLayout(int numRows, int numColumns);

    // setup
    void setMarginX(int margin);
    void setMarginY(int margin);
    void setFillRow(int rowIndex);
    void setFillColumn(int columnIndex);
    void setFixedSizeRow(int rowIndex, int height);
    void setFixedSizeColumn(int columnIndex, int width);
    void setRowSpacing(int spacing);
    void setColumnSpacing(int spacing);
    TView* add(int rowIndex, int columnIndex, TView* view);
    void add(int rowIndex, int columnIndex, FlowLayout* flow, bool leftAlign);

    // run
    TPoint apply(TRect extent);
    void apply(TWindow* window);

   private:
    struct TableView {
        int rowIndex;
        int columnIndex;
        // one or the other: view or flow
        TView* view;
        FlowLayout* flow;
        bool flowLeftAlign;
    };

    TableView getCell(int rowIndex, int columnIndex);
    void calculateRowHeights(int finalRowHeights[], const TRect& extent);
    void calculateColumnWidths(int finalColumnWidths[], const TRect& extent);

    int _marginX = 3;
    int _marginY = 2;
    std::vector<int> _rowHeights;    // -1=fill, 0=auto
    std::vector<int> _columnWidths;  // -1=fill, 0=auto
    int _rowSpacing = 1;
    int _columnSpacing = 1;
    std::vector<TableView> _views;
};

}  // namespace tmbasic
