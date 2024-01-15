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

// (red as Number, green as Number, blue as Number)
void systemCallRgb(const SystemCallInput& input, SystemCallResult* result) {
    auto red = input.getValue(-3).num;
    if (red < 0 || red >= 256) {
        throw Error(ErrorCode::kInvalidArgument, "Red must be between 0 and 255.");
    }

    auto green = input.getValue(-2).num;
    if (green < 0 || green >= 256) {
        throw Error(ErrorCode::kInvalidArgument, "Green must be between 0 and 255.");
    }

    auto blue = input.getValue(-1).num;
    if (blue < 0 || blue >= 256) {
        throw Error(ErrorCode::kInvalidArgument, "Blue must be between 0 and 255.");
    }

    RecordBuilder builder{ 3, 0 };
    builder.values.set(0, Value{ red.floor() });
    builder.values.set(1, Value{ green.floor() });
    builder.values.set(2, Value{ blue.floor() });
    result->returnedObject = boost::make_local_shared<Record>(&builder);
}

}  // namespace vm
