#include "systemCall.h"
#include "BasicLabel.h"
#include "BasicFormsStorage.h"
#include "Error.h"
#include "String.h"
#include "util/WindowPtr.h"

using util::WindowPtr;

namespace vm {

void initSystemCallsControls() {
    initSystemCall(SystemCall::kNewLabel, [](const auto& input, auto* result) {
        result->returnedValue.num = BasicLabel::newLabel(input.interpreter);
    });

    initSystemCall(SystemCall::kControlText, [](const auto& input, auto* result) {
        auto* control = basicFormsStorage.controls.find(input.getValue(-1).getInt64());

        auto* label = dynamic_cast<BasicLabel*>(control);
        if (label != nullptr) {
            result->returnedObject = label->getText();
            return;
        }

        throw Error(ErrorCode::kInvalidControlType, "ControlText does not support this type of control.");
    });

    initSystemCall(SystemCall::kSetControlText, [](const auto& input, auto* result) {
        auto* control = basicFormsStorage.controls.find(input.getValue(-1).getInt64());
        auto& text = dynamic_cast<String&>(input.getObject(-1));

        auto* label = dynamic_cast<BasicLabel*>(control);
        if (label != nullptr) {
            label->setText(text);
            return;
        }

        throw Error(ErrorCode::kInvalidControlType, "SetControlText does not support this type of control.");
    });
}

}  // namespace vm
