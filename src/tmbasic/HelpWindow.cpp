#include "HelpWindow.h"
#include "../shared/ScrollBar.h"
#include "../shared/ViewPtr.h"
#include "App.h"

namespace tmbasic {

class HelpWindowFrame : public TFrame {
   public:
    explicit HelpWindowFrame(TRect r) : TFrame(r) {}
    TColorAttr mapColor(uchar index) noexcept override {
        auto attr = TFrame::mapColor(index);
        auto fore = getFore(attr);
        auto back = getBack(attr);
        if (back == TColorBIOS(0x3)) {
            back = TColorRGB(235, 235, 235);  // cyan -> off-white
        }
        if (fore == TColorBIOS(0xF)) {
            fore = TColorBIOS(0);  // white -> black
        } else if (fore == TColorBIOS(0xA)) {
            fore = TColorBIOS(0x2);  // light green -> dark green
        }
        return { fore, back };
    }

    static TFrame* init(TRect r) { return new HelpWindowFrame(r); }
};

class HelpViewer : public THelpViewer {
   public:
    HelpViewer(
        const TRect& bounds,
        TScrollBar* aHScrollBar,
        TScrollBar* aVScrollBar,
        THelpFile* aHelpFile,
        ushort context)
        : THelpViewer(bounds, aHScrollBar, aVScrollBar, aHelpFile, context) {}

    TColorAttr mapColor(uchar index) noexcept override {
        auto attr = THelpViewer::mapColor(index);
        auto fore = getFore(attr);
        auto back = getBack(attr);
        if (back == TColorBIOS(0x3)) {
            back = TColorRGB(235, 235, 235);  // cyan -> off-white
        } else if (back == TColorBIOS(0x1)) {
            back = TColorRGB(255, 255, 255);  // dark blue -> white
        }
        if (fore == TColorBIOS(0xF)) {
            fore = TColorBIOS(0);  // white -> black
        } else if (fore == TColorBIOS(0xE)) {
            fore = TColorBIOS(0x2);  // yellow -> dark green
        }
        return { fore, back };
    }
};

static shared::ViewPtr<shared::ScrollBar> createScrollBar(TRect r, ushort aOptions) {
    if (aOptions & sbVertical) {
        r = TRect(r.b.x - 1, r.a.y + 1, r.b.x, r.b.y - 1);
    } else {
        r = TRect(r.a.x + 2, r.b.y - 1, r.b.x - 2, r.b.y);
    }

    shared::ViewPtr<shared::ScrollBar> s{ r };
    if (aOptions & sbHandleKeyboard) {
        s->options |= ofPostProcess;
    }
    return s;
}

HelpWindow::HelpWindow(THelpFile* hFile, ushort context)
    : TWindow(TRect(0, 0, 60, 18), "Help", wnNoNumber), TWindowInit(HelpWindowFrame::init) {
    TRect r{ 0, 0, 60, 18 };
    options = (options | ofCentered);
    r.grow(-2, -1);

    auto hScrollbar = createScrollBar(getExtent(), sbHorizontal | sbHandleKeyboard);
    hScrollbar->useWhiteColorScheme();
    hScrollbar.addTo(this);
    auto vScrollbar = createScrollBar(getExtent(), sbVertical | sbHandleKeyboard);
    vScrollbar->useWhiteColorScheme();
    vScrollbar.addTo(this);

    shared::ViewPtr<HelpViewer> viewer(r, hScrollbar, vScrollbar, hFile, context);
    viewer.addTo(this);
}

TPalette& HelpWindow::getPalette() const {
    static TPalette palette(App::helpWindowPalette.data(), App::helpWindowPalette.size() - 1);
    return palette;
}

}  // namespace tmbasic
