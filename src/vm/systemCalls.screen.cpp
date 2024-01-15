#include "vm/systemCall.h"
#include "vm/BasicApp.h"
#include "vm/castObject.h"

namespace vm {

// ()
void systemCallEnterFullscreen(const SystemCallInput& /*input*/, SystemCallResult* /*result*/) {
    BasicApp::createInstance();
}

// ()
void systemCallFlushConsoleOutput(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto* app = BasicApp::instance.get();
    if (app != nullptr) {
        // Full-screen mode
        app->redraw();
        TScreen::flushScreen();
        TEvent event{};
        app->getEvent(event);
    } else {
        // Command line mode
        input.consoleOutputStream->flush();
    }
}

// (input as String)
void systemCallPrintString(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto* app = BasicApp::instance.get();
    const auto& str = castString(input.getObject(-1))->value;
    if (app != nullptr) {
        // Full-screen mode
        app->console->print(str);
    } else {
        // Command line mode
        *input.consoleOutputStream << str;
    }
}

}  // namespace vm
