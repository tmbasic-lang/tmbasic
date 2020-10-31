#include "ProgramWindow.h"

namespace ui {

ProgramWindow::ProgramWindow(const TRect& r)
    : TWindow(r, "Untitled (program)", wnNoNumber), TWindowInit(TWindow::initFrame) {}

}  // namespace ui
