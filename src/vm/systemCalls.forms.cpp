#include "vm/systemCall.h"
#include "vm/BasicForm.h"
#include "vm/BasicFormsStorage.h"
#include "vm/Error.h"
#include "vm/castObject.h"
#include "vm/String.h"
#include "util/WindowPtr.h"

namespace vm {

class BasicApp : public TApplication {
   public:
    BasicApp() : TProgInit(initStatusLine, initMenuBar, initDeskTop) {}
};

void initSystemCallsForms() {
    initSystemCall(SystemCall::kNewForm, [](const auto& input, auto* result) {
        result->returnedValue.num = BasicForm::newForm(input.interpreter);
    });

    initSystemCall(SystemCall::kRunForm, [](const auto& input, auto* /*result*/) {
        auto* form = basicFormsStorage.forms.find(input.getValue(-1).getInt64());
        assert(form != nullptr);
        assert(TProgram::application == nullptr);
        BasicApp app{};
        TProgram::deskTop->insert(form);
        app.run();
        app.shutDown();
    });

    initSystemCall(SystemCall::kFormTitle, [](const auto& input, auto* result) {
        auto* form = basicFormsStorage.forms.find(input.getValue(-1).getInt64());
        std::string title{ form->title };
        result->returnedObject = boost::make_local_shared<String>(std::move(title));
    });

    initSystemCall(SystemCall::kSetFormTitle, [](const auto& input, auto* /*result*/) {
        auto* form = basicFormsStorage.forms.find(input.getValue(-1).getInt64());
        auto& title = castString(input.getObject(-1));
        delete[] form->title;  // NOLINT(cppcoreguidelines-owning-memory)
        form->title = newStr(title.value);
    });

    initSystemCall(SystemCall::kAddControlToForm, [](const auto& input, auto* /*result*/) {
        auto* form = basicFormsStorage.forms.find(input.getValue(-2).getInt64());
        auto* control = basicFormsStorage.controls.find(input.getValue(-1).getInt64());
        if (control->owner != nullptr) {
            throw Error(ErrorCode::kInvalidOperation, "This control was already added to another form.");
        }
        form->insert(control);
    });
}

}  // namespace vm
