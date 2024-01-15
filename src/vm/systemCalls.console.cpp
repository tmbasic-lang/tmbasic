#include "vm/systemCall.h"
#include "vm/BasicApp.h"
#include "vm/castObject.h"

using shared::Error;
using shared::ErrorCode;

namespace vm {

static TColorRGB colorRecordToColorRgb(const Record& colorRecord) {
    auto red = static_cast<uint8_t>(colorRecord.values.at(0).getInt32());
    auto green = static_cast<uint8_t>(colorRecord.values.at(1).getInt32());
    auto blue = static_cast<uint8_t>(colorRecord.values.at(2).getInt32());
    return { red, green, blue };
}

static boost::local_shared_ptr<Record> colorRgbToColorRecord(const TColorRGB& colorRgb) {
    RecordBuilder builder{ 3, 0 };
    builder.values.set(0, Value{ colorRgb.r });
    builder.values.set(1, Value{ colorRgb.g });
    builder.values.set(2, Value{ colorRgb.b });
    return boost::make_local_shared<Record>(&builder);
}

static TColorRGB colorDesiredToColorRgb(const TColorDesired& colorDesired) {
    if (colorDesired.isRGB()) {
        return colorDesired.asRGB();
    }

    if (colorDesired.isBIOS()) {
        auto bios = colorDesired.asBIOS();
        TColorRGB rgb{};
        if (bios.r) {
            rgb.r = bios.bright ? 255 : 128;
        }
        if (bios.g) {
            rgb.g = bios.bright ? 255 : 128;
        }
        if (bios.b) {
            rgb.b = bios.bright ? 255 : 128;
        }
        return rgb;
    }

    throw Error(ErrorCode::kInternalTypeConfusion, "Unexpected color type.");
}

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

template <decltype(&setFore) setForeOrBack>
static void setConsoleColor(const SystemCallInput& input) {
    auto* app = BasicApp::instance.get();
    if (app == nullptr) {
        throw Error(ErrorCode::kWrongScreenMode, "Must be in fullscreen mode.");
    }

    const auto& colorRecord = castRecord(input.getObject(-1));
    const auto colorRgb = colorRecordToColorRgb(*colorRecord);
    setForeOrBack(app->console->currentColorAttr, colorRgb);
}

// (color as Color)
void systemCallSetConsoleForeColor(const SystemCallInput& input, SystemCallResult* /*result*/) {
    setConsoleColor<setFore>(input);
}

// (color as Color)
void systemCallSetConsoleBackColor(const SystemCallInput& input, SystemCallResult* /*result*/) {
    setConsoleColor<setBack>(input);
}

template <decltype(&getFore) getForeOrBack>
static void getConsoleColor(SystemCallResult* result) {
    auto* app = BasicApp::instance.get();
    if (app == nullptr) {
        throw Error(ErrorCode::kWrongScreenMode, "Must be in fullscreen mode.");
    }

    const auto colorDesired = getForeOrBack(app->console->currentColorAttr);
    auto rgb = colorDesiredToColorRgb(colorDesired);
    result->returnedObject = colorRgbToColorRecord(rgb);
}

// () as Color
void systemCallConsoleForeColor(const SystemCallInput& /*input*/, SystemCallResult* result) {
    getConsoleColor<getFore>(result);
}

// () as Color
void systemCallConsoleBackColor(const SystemCallInput& /*input*/, SystemCallResult* result) {
    getConsoleColor<getBack>(result);
}

}  // namespace vm
