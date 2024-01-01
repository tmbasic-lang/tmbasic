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

void systemCallNewForm(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = BasicForm::newForm(input.interpreter);
}

void systemCallRunForm(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto* form = basicFormsStorage.forms.find(input.getValue(-1).getInt64());
    assert(form != nullptr);
    assert(TProgram::application == nullptr);
    BasicApp app{};
    TProgram::deskTop->insert(form);
    app.run();
    app.shutDown();
}

void systemCallFormTitle(const SystemCallInput& input, SystemCallResult* result) {
    auto* form = basicFormsStorage.forms.find(input.getValue(-1).getInt64());
    std::string title{ form->title };
    result->returnedObject = boost::make_local_shared<String>(std::move(title));
}

void systemCallSetFormTitle(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto* form = basicFormsStorage.forms.find(input.getValue(-1).getInt64());
    auto& title = *castString(input.getObject(-1));
    delete[] form->title;  // NOLINT(cppcoreguidelines-owning-memory)
    form->title = newStr(title.value);
}

void systemCallAddControlToForm(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto* form = basicFormsStorage.forms.find(input.getValue(-2).getInt64());
    auto* control = basicFormsStorage.controls.find(input.getValue(-1).getInt64());
    if (control->owner != nullptr) {
        throw Error(ErrorCode::kInvalidOperation, "This control was already added to another form.");
    }
    form->insert(control);
}

}  // namespace vm
