#include "vm/systemCall.h"
#include "vm/BasicApp.h"
#include "vm/castObject.h"

using shared::Error;
using shared::ErrorCode;

namespace vm {

static void validateAndFloorRgb(Value* red, Value* green, Value* blue) {
    if (red->num < 0 || red->num >= 256) {
        throw Error(ErrorCode::kInvalidArgument, "Red must be between 0 and 255.");
    }
    red->num = red->num.floor();

    if (green->num < 0 || green->num >= 256) {
        throw Error(ErrorCode::kInvalidArgument, "Green must be between 0 and 255.");
    }
    green->num = green->num.floor();

    if (blue->num < 0 || blue->num >= 256) {
        throw Error(ErrorCode::kInvalidArgument, "Blue must be between 0 and 255.");
    }
    blue->num = blue->num.floor();
}

static TColorRGB colorRecordToColorRgb(const Record& colorRecord) {
    auto red = colorRecord.values.at(0);
    auto green = colorRecord.values.at(1);
    auto blue = colorRecord.values.at(2);
    validateAndFloorRgb(&red, &green, &blue);
    return { static_cast<uint8_t>(red.getInt32()), static_cast<uint8_t>(green.getInt32()),
             static_cast<uint8_t>(blue.getInt32()) };
}

static boost::intrusive_ptr<Record> colorRgbToColorRecord(const TColorRGB& colorRgb) {
    RecordBuilder builder{ 3, 0 };
    builder.values.set(0, Value{ colorRgb.r });
    builder.values.set(1, Value{ colorRgb.g });
    builder.values.set(2, Value{ colorRgb.b });
    return boost::make_intrusive_ptr<Record>(&builder);
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
    auto red = input.getValue(-3);
    auto green = input.getValue(-2);
    auto blue = input.getValue(-1);
    validateAndFloorRgb(&red, &green, &blue);

    RecordBuilder builder{ 3, 0 };
    builder.values.set(0, red);
    builder.values.set(1, green);
    builder.values.set(2, blue);
    result->returnedObject = boost::make_intrusive_ptr<Record>(&builder);
}

template <decltype(&setFore) setForeOrBack>
static void setConsoleColorRgb(const SystemCallInput& input) {
    auto* app = BasicApp::instance.get();
    if (app == nullptr) {
        throw Error(ErrorCode::kWrongScreenMode, "Must be in fullscreen mode.");
    }

    const auto& colorRecord = castRecord(input.getObject(-1));
    const auto colorRgb = colorRecordToColorRgb(*colorRecord);
    setForeOrBack(app->console->currentColorAttr, colorRgb);
}

// (color as Color)
void systemCallSetForeColor(const SystemCallInput& input, SystemCallResult* /*result*/) {
    setConsoleColorRgb<setFore>(input);
}

// (color as Color)
void systemCallSetBackColor(const SystemCallInput& input, SystemCallResult* /*result*/) {
    setConsoleColorRgb<setBack>(input);
}

template <decltype(&setFore) setForeOrBack>
static void setConsoleColorComponents(const SystemCallInput& input) {
    auto* app = BasicApp::instance.get();
    if (app == nullptr) {
        throw Error(ErrorCode::kWrongScreenMode, "Must be in fullscreen mode.");
    }

    auto red = input.getValue(-3);
    auto green = input.getValue(-2);
    auto blue = input.getValue(-1);
    validateAndFloorRgb(&red, &green, &blue);

    TColorRGB colorRgb{ static_cast<uint8_t>(red.getInt32()), static_cast<uint8_t>(green.getInt32()),
                        static_cast<uint8_t>(blue.getInt32()) };
    setForeOrBack(app->console->currentColorAttr, colorRgb);
}

// (red as Number, green as Number, blue as Number)
void systemCallSetForeColorComponents(const SystemCallInput& input, SystemCallResult* /*result*/) {
    setConsoleColorComponents<setFore>(input);
}

// (red as Number, green as Number, blue as Number)
void systemCallSetBackColorComponents(const SystemCallInput& input, SystemCallResult* /*result*/) {
    setConsoleColorComponents<setBack>(input);
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
void systemCallForeColor(const SystemCallInput& /*input*/, SystemCallResult* result) {
    getConsoleColor<getFore>(result);
}

// () as Color
void systemCallBackColor(const SystemCallInput& /*input*/, SystemCallResult* result) {
    getConsoleColor<getBack>(result);
}

// (x as Number, y as Number)
void systemCallMoveCursor(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto* app = BasicApp::instance.get();
    if (app == nullptr) {
        throw Error(ErrorCode::kWrongScreenMode, "Must be in fullscreen mode.");
    }

    auto x = input.getValue(-2);
    if (x.num < 0 || x.num >= 32768) {
        throw Error(ErrorCode::kInvalidArgument, "X must be between 0 and 32767.");
    }

    auto y = input.getValue(-1);
    if (y.num < 0 || y.num >= 32768) {
        throw Error(ErrorCode::kInvalidArgument, "Y must be between 0 and 32767.");
    }

    app->console->currentX = static_cast<int16_t>(x.getInt32());
    app->console->currentY = static_cast<int16_t>(y.getInt32());
}

// () as Number
void systemCallScreenWidth(const SystemCallInput& /*input*/, SystemCallResult* result) {
    auto* app = BasicApp::instance.get();
    if (app == nullptr) {
        throw Error(ErrorCode::kWrongScreenMode, "Must be in fullscreen mode.");
    }

    result->returnedValue = Value{ THardwareInfo::getScreenCols() };
}

// () as Number
void systemCallScreenHeight(const SystemCallInput& /*input*/, SystemCallResult* result) {
    auto* app = BasicApp::instance.get();
    if (app == nullptr) {
        throw Error(ErrorCode::kWrongScreenMode, "Must be in fullscreen mode.");
    }

    result->returnedValue = Value{ THardwareInfo::getScreenRows() };
}

// ()
void systemCallCls(const SystemCallInput& /*input*/, SystemCallResult* /*result*/) {
    auto* app = BasicApp::instance.get();
    if (app == nullptr) {
        throw Error(ErrorCode::kWrongScreenMode, "Must be in fullscreen mode.");
    }

    app->console->cells.clear();
    app->console->fillColor = getBack(app->console->currentColorAttr).asRGB();
    if (!app->console->isBuffered) {
        app->forceScreenUpdate();
    }
}

}  // namespace vm
