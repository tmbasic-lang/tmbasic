#include "FlowLayout.h"

namespace tmbasic {

FlowLayout::FlowLayout() {}

void FlowLayout::setVerticalOrientation() {
    _horizontal = false;
}

void FlowLayout::setHorizontalOrientation() {
    _horizontal = true;
}

void FlowLayout::setMarginX(int margin) {
    _marginX = margin;
    _width = margin * 2;
}

void FlowLayout::setMarginY(int margin) {
    _marginY = margin;
    _height = margin * 2;
}

void FlowLayout::setItemSpacing(int spacing) {
    _itemSpacing = spacing;
}

TView* FlowLayout::add(TView* view) {
    _views.push_back(view);
    if (_horizontal) {
        _width += view->size.x;
        _height = max(_height, view->size.y);
        if (_views.size() >= 2) {
            _width += _itemSpacing;
        }
    } else {
        _width = max(_width, view->size.x);
        _height += view->size.y;
        if (_views.size() >= 2) {
            _height += _itemSpacing;
        }
    }
    return view;
}

int FlowLayout::getHeight() {
    return _height;
}

int FlowLayout::getWidth() {
    return _width;
}

void FlowLayout::applyLeftAligned(int left, int top) {
    auto x = left + _marginX;
    auto y = top + _marginY;

    for (auto* view : _views) {
        auto width = view->size.x - 1;
        auto height = view->size.y;

        TRect r(x, y, x + width, y + height);
        view->locate(r);

        if (_horizontal) {
            x += width + _itemSpacing;
        } else {
            y += height + _itemSpacing;
        }
    }
}

void FlowLayout::applyRightAligned(int right, int top) {
    auto x = right - _marginX;
    auto y = top + _marginY;

    for (size_t i = 0; i < _views.size(); i++) {
        auto* view = _views[_views.size() - 1 - i];
        auto width = view->size.x;
        auto height = view->size.y;

        TRect r(x - width + 1, y, x, y + height);
        view->locate(r);

        if (_horizontal) {
            x -= width + _itemSpacing - 1;
        } else {
            y -= height + _itemSpacing;
        }
    }
}

}  // namespace tmbasic
