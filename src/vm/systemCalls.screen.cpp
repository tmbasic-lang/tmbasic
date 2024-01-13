#include "vm/systemCall.h"
#include "vm/BasicApp.h"
#include "vm/castObject.h"

namespace vm {

static BasicApp* getApp() {
    return dynamic_cast<BasicApp*>(TProgram::application);
}

// ()
void systemCallEnterFullscreenMode(const SystemCallInput& /*input*/, SystemCallResult* /*result*/) {
    auto* app = getApp();
    app->resume();
    app->redraw();
}

// ()
void systemCallEnterCommandLineMode(const SystemCallInput& /*input*/, SystemCallResult* /*result*/) {
    getApp()->suspend();
}

// (input as String)
void systemCallPrintString(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto* app = getApp();
    const auto& str = castString(input.getObject(-1))->value;
    if (app != nullptr && app->isActive()) {
        // Full-screen mode
        app->background->print(str);
        app->drawView();
        app->redraw();
        TScreen::flushScreen();
    } else {
        // Command line mode
        *input.consoleOutputStream << str;
    }
}

}  // namespace vm
