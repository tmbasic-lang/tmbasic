#pragma once

#include "../common.h"

namespace tmbasic {

class FlowLayout {
   public:
    FlowLayout();

    // setup
    void setVerticalOrientation();
    void setHorizontalOrientation();
    void setMarginX(int margin);
    void setMarginY(int margin);
    void setItemSpacing(int spacing);
    TView* add(TView* view);

    // run
    int getHeight();
    int getWidth();
    void applyLeftAligned(int left, int top);
    void applyRightAligned(int right, int top);

   private:
    bool _horizontal = true;
    int _marginX = 0;
    int _marginY = 0;
    int _itemSpacing = 1;
    std::vector<TView*> _views;
    int _height = 0;
    int _width = 0;
};

}  // namespace tmbasic
