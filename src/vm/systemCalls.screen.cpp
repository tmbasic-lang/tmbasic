#include "vm/systemCall.h"
#include "vm/BasicApp.h"
#include "vm/castObject.h"

using shared::Error;
using shared::ErrorCode;

namespace vm {

// ()
void systemCallEnterFullscreen(const SystemCallInput& /*input*/, SystemCallResult* /*result*/) {
    BasicApp::createInstance();
}

// (enable as Boolean)
void systemCallSetFullscreenBuffering(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto enable = input.getValue(-1).getBoolean();

    auto* app = BasicApp::instance.get();
    if (app == nullptr) {
        throw Error(ErrorCode::kWrongScreenMode, "Must be in fullscreen mode.");
    }

    app->console->isBuffered = enable;

    // If we just disabled buffering, then redraw the screen right now.
    if (!enable) {
        app->forceScreenUpdate();
    }
}

// ()
void systemCallUpdateScreen(const SystemCallInput& /*input*/, SystemCallResult* /*result*/) {
    auto* app = BasicApp::instance.get();
    if (app == nullptr) {
        throw Error(ErrorCode::kWrongScreenMode, "Must be in fullscreen mode.");
    }

    if (!app->console->isBuffered) {
        throw Error(ErrorCode::kInvalidOperation, "Screen buffering is not enabled.");
    }

    app->forceScreenUpdate();
}

// ()
void systemCallFlushConsoleOutput(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto* app = BasicApp::instance.get();
    if (app != nullptr) {
        // Full-screen mode. In buffered mode, this is a noop.
        if (!app->console->isBuffered) {
            app->forceScreenUpdate();
        }
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
