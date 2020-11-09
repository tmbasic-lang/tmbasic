#include "ProgramWindow.h"

namespace tmbasic {

ProgramWindow::ProgramWindow(const TRect& r)
    : TWindow(r, "Untitled (program)", wnNoNumber), TWindowInit(TWindow::initFrame) {}

}  // namespace tmbasic
