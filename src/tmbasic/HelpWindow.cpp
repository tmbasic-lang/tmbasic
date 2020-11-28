#include "HelpWindow.h"
#include "App.h"

namespace tmbasic {

HelpWindow::HelpWindow(THelpFile* hFile, ushort context)
    : THelpWindow(hFile, context), TWindowInit(&THelpWindow::initFrame) {}

TPalette& HelpWindow::getPalette() const {
    static TPalette palette(App::helpWindowPalette, sizeof(App::helpWindowPalette) - 1);
    return palette;
}

}  // namespace tmbasic
