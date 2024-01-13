#include "BasicApp.h"
#include "BasicBackground.h"

namespace vm {

class DeskTop : public TDeskTop {
   public:
    explicit DeskTop(const TRect& r) : TDeskTop(r), TDeskInit(initBasicBackground) {}
    static TBackground* initBasicBackground(TRect r) { return new BasicBackground(r); }
};

BasicApp::BasicApp()
    : TProgInit(initBasicStatusLine, initBasicMenuBar, initBasicDeskTop),
      background(dynamic_cast<BasicBackground*>(deskTop->background)) {
    assert(background != nullptr);
}

TStatusLine* BasicApp::initBasicStatusLine(TRect r) {
    return nullptr;
}

TMenuBar* BasicApp::initBasicMenuBar(TRect r) {
    return nullptr;
}

TDeskTop* BasicApp::initBasicDeskTop(TRect r) {
    return new DeskTop(r);
}

void BasicApp::run() {
    if (!_active) {
        _active = true;
        TApplication::run();
    }
}

void BasicApp::suspend() {
    if (_active) {
        _active = false;
        TApplication::suspend();
    }
}

void BasicApp::resume() {
    if (!_active) {
        _active = true;
        TApplication::resume();
    }
}

bool BasicApp::isActive() {
    return _active;
}

}  // namespace vm
