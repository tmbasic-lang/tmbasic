#pragma once

#include "../common.h"

namespace util {

class Editor : public TEditor {
   public:
    Editor(
        const TRect& bounds,
        TScrollBar* aHScrollBar,
        TScrollBar* aVScrollBar,
        TIndicator* aIndicator,
        uint aBufSize);
    std::string getText() const;
    void setText(const std::string& str);
};

}  // namespace util
