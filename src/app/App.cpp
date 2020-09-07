#include "App.h"

App::App(int argc, char** argv) : TProgInit(initStatusLine, initMenuBar, TApplication::initDeskTop) {}

void App::handleEvent(TEvent& event) {
    TApplication::handleEvent(event);
}

TMenuBar* App::initMenuBar(TRect r) {
    r.b.y = r.a.y + 1;

    return new TMenuBar(
        r, *new TSubMenu("~H~ello", kbAltH) + *new TMenuItem("E~x~it", cmQuit, cmQuit, hcNoContext, "Alt-X"));
}

TStatusLine* App::initStatusLine(TRect r) {
    r.a.y = r.b.y - 1;
    return new TStatusLine(
        r,
        *new TStatusDef(0, 0xFFFF) + *new TStatusItem("~Alt-X~ Exit", kbAltX, cmQuit) +
            *new TStatusItem(0, kbF10, cmMenu));
}
