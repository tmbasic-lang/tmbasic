#include "RowLayout.h"

namespace tmbasic {

RowLayout::RowLayout(bool rightAlign) : _rightAlign(rightAlign) {}

RowLayout::RowLayout(bool rightAlign, std::initializer_list<TView*> views) : _rightAlign(rightAlign) {
    for (auto* view : views) {
        add(view);
    }
}

RowLayout& RowLayout::setVerticalOrientation() {
    _horizontal = false;
    return *this;
}

RowLayout& RowLayout::setHorizontalOrientation() {
    _horizontal = true;
    return *this;
}

RowLayout& RowLayout::setMarginX(int margin) {
    _marginX = margin;
    return *this;
}

RowLayout& RowLayout::setMarginY(int margin) {
    _marginY = margin;
    return *this;
}

RowLayout& RowLayout::setItemSpacing(int spacing) {
    _itemSpacing = spacing;
    return *this;
}

TView* RowLayout::add(TView* view) {
    _views.push_back(view);
    return view;
}

TPoint RowLayout::getSize() {
    auto width = _marginX * 2;
    auto height = _marginY * 2;

    for (auto* view : _views) {
        if (_horizontal) {
            width += view->size.x;
            height = max(height, view->size.y);
            if (_views.size() >= 2) {
                width += _itemSpacing;
            }
        } else {
            width = max(width, view->size.x);
            height += view->size.y;
            if (_views.size() >= 2) {
                height += _itemSpacing;
            }
        }
    }

    return TPoint{ width, height };
}

void RowLayout::addLeftAlignedTo(TGroup* group, int left, int top) {
    auto x = left + _marginX;
    auto y = top + _marginY;

    for (auto* view : _views) {
        auto width = view->size.x - 1;
        auto height = view->size.y;

        TRect r(x, y, x + width, y + height);
        view->locate(r);
        group->insert(view);

        if (_horizontal) {
            x += width + _itemSpacing;
        } else {
            y += height + _itemSpacing;
        }
    }
}

void RowLayout::addRightAlignedTo(TGroup* group, int right, int top) {
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

    // for the purpose of tab ordering, insert the views in left-to-right order
    for (auto* view : _views) {
        group->insert(view);
    }
}

void RowLayout::addTo(TGroup* group, int left, int right, int top) {
    if (_rightAlign) {
        addRightAlignedTo(group, right, top);
    } else {
        addLeftAlignedTo(group, left, top);
    }
}

}  // namespace tmbasic
