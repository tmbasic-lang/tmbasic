#include "BasicApp.h"
#include "BasicConsoleView.h"

namespace vm {

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::unique_ptr<BasicApp> BasicApp::instance{};

class DeskTop : public TDeskTop {
   public:
    explicit DeskTop(const TRect& r) : TDeskTop(r), TDeskInit(initBasicBackground) {}
    static gsl::owner<TBackground*> initBasicBackground(TRect r) { return new TBackground(r, ' '); }
};

BasicApp* BasicApp::createInstance() {
    if (instance == nullptr) {
        instance = std::make_unique<BasicApp>();
    }
    return instance.get();
}

BasicApp::BasicApp() : TProgInit(initBasicStatusLine, initBasicMenuBar, initBasicDeskTop), console(getExtent()) {
    console.addTo(this);
}

BasicApp::~BasicApp() {
    shutDown();
}

TStatusLine* BasicApp::initBasicStatusLine(TRect /*r*/) {
    return nullptr;
}

TMenuBar* BasicApp::initBasicMenuBar(TRect /*r*/) {
    return nullptr;
}

TDeskTop* BasicApp::initBasicDeskTop(TRect r) {
    return new DeskTop(r);
}

}  // namespace vm
