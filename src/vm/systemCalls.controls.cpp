#include "vm/systemCall.h"
#include "util/WindowPtr.h"
#include "vm/BasicFormsStorage.h"
#include "vm/BasicLabel.h"
#include "vm/castObject.h"
#include "vm/Error.h"
#include "vm/Record.h"
#include "vm/RecordBuilder.h"
#include "vm/String.h"

namespace vm {

static boost::local_shared_ptr<Record> newRectangle(int left, int top, int width, int height);
static TRect newTRect(int left, int top, int width, int height);

void initSystemCallsControls() {
    initSystemCall(SystemCall::kNewLabel, [](const auto& input, auto* result) {
        result->returnedValue.num = BasicLabel::newLabel(input.interpreter);
    });

    initSystemCall(SystemCall::kControlText, [](const auto& input, auto* result) {
        auto* control = basicFormsStorage.controls.find(input.getValue(-1).getInt64());

        auto* label = dynamic_cast<BasicLabel*>(control);
        if (label != nullptr) {
            result->returnedObject = label->getBasicText();
            return;
        }

        throw Error(ErrorCode::kInvalidControlType, "ControlText does not support this type of control.");
    });

    initSystemCall(SystemCall::kSetControlText, [](const auto& input, auto* /*result*/) {
        auto* control = basicFormsStorage.controls.find(input.getValue(-1).getInt64());
        auto& text = *castString(input.getObject(-1));

        auto* label = dynamic_cast<BasicLabel*>(control);
        if (label != nullptr) {
            label->setText(text);
            return;
        }

        throw Error(ErrorCode::kInvalidControlType, "SetControlText does not support this type of control.");
    });

    initSystemCall(SystemCall::kControlBounds, [](const auto& input, auto* result) {
        auto* control = basicFormsStorage.controls.find(input.getValue(-1).getInt64());
        auto tvRect = control->getBounds();
        result->returnedObject = newRectangle(tvRect.a.x, tvRect.a.y, tvRect.b.x - tvRect.a.x, tvRect.b.y - tvRect.a.y);
    });

    initSystemCall(SystemCall::kSetControlBounds1, [](const auto& input, auto* /*result*/) {
        auto* control = basicFormsStorage.controls.find(input.getValue(-1).getInt64());
        const auto& rect = dynamic_cast<const Record&>(*input.getObject(-1));
        control->setBounds(newTRect(
            rect.values.at(0).getInt32(), rect.values.at(1).getInt32(), rect.values.at(2).getInt32(),
            rect.values.at(3).getInt32()));
        if (control->owner != nullptr) {
            control->owner->drawView();
        }
    });

    initSystemCall(SystemCall::kSetControlBounds2, [](const auto& input, auto* /*result*/) {
        auto* control = basicFormsStorage.controls.find(input.getValue(-5).getInt64());
        control->setBounds(newTRect(
            input.getValue(-4).getInt32(), input.getValue(-3).getInt32(), input.getValue(-2).getInt32(),
            input.getValue(-1).getInt32()));
        if (control->owner != nullptr) {
            control->owner->drawView();
        }
    });
}

/* static */ boost::local_shared_ptr<Record> newRectangle(int left, int top, int width, int height) {
    RecordBuilder builder{ 4, 0 };
    builder.values.set(0, Value{ left });
    builder.values.set(1, Value{ top });
    builder.values.set(2, Value{ width });
    builder.values.set(3, Value{ height });
    return boost::make_local_shared<Record>(&builder);
}

/* static */ TRect newTRect(int left, int top, int width, int height) {
    return TRect{ left, top, left + width, top + height };
}

}  // namespace vm
