#pragma once

#include "../common.h"

namespace tmbasic {

class RowLayout {
   public:
    explicit RowLayout(bool rightAlign);
    RowLayout(bool rightAlign, std::initializer_list<TView*> views);

    // setup
    RowLayout& setVerticalOrientation();
    RowLayout& setHorizontalOrientation();
    RowLayout& setMarginX(int margin);
    RowLayout& setMarginY(int margin);
    RowLayout& setItemSpacing(int spacing);
    TView* add(TView* view);

    // run
    TPoint getSize();
    void addTo(TGroup* group, int left, int right, int top);

   private:
    void addLeftAlignedTo(TGroup* group, int left, int top);
    void addRightAlignedTo(TGroup* group, int right, int top);

    bool _rightAlign = false;
    bool _horizontal = true;
    int _marginX = 0;
    int _marginY = 0;
    int _itemSpacing = 1;
    std::vector<TView*> _views;
};

}  // namespace tmbasic
