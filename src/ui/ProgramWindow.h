#pragma once
#define Uses_TRect
#define Uses_TWindow
#include <tvision/tv.h>

namespace ui {

class ProgramWindow : public TWindow {
   public:
    ProgramWindow(const TRect& r);
};

}  // namespace ui
