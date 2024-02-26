#include "vm/systemCall.h"
#include "shared/Error.h"
#include "shared/WindowPtr.h"
#include "vm/BasicApp.h"
#include "vm/BasicForm.h"
#include "vm/BasicFormsStorage.h"
#include "vm/castObject.h"
#include "vm/String.h"

using shared::Error;
using shared::ErrorCode;

namespace vm {

// () as Form
void systemCallNewForm(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = BasicForm::newForm(input.interpreter);
}

// (form as Form)
void systemCallRunForm(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto* form = basicFormsStorage.forms.find(input.getValue(-1).getInt64());
    assert(form != nullptr);
    TProgram::deskTop->insert(form);
    TProgram::application->run();
}

// (form as Form) as String
void systemCallFormTitle(const SystemCallInput& input, SystemCallResult* result) {
    auto* form = basicFormsStorage.forms.find(input.getValue(-1).getInt64());
    std::string title{ form->title };
    result->returnedObject = boost::make_intrusive_ptr<String>(std::move(title));
}

// (form as Form, title as String)
void systemCallSetFormTitle(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto* form = basicFormsStorage.forms.find(input.getValue(-1).getInt64());
    auto& title = *castString(input.getObject(-1));
    delete[] form->title;  // NOLINT(cppcoreguidelines-owning-memory)
    form->title = newStr(title.value);
}

// (form as Form, control as Control)
void systemCallAddControlToForm(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto* form = basicFormsStorage.forms.find(input.getValue(-2).getInt64());
    auto* control = basicFormsStorage.controls.find(input.getValue(-1).getInt64());
    if (control->owner != nullptr) {
        throw Error(ErrorCode::kInvalidOperation, "This control was already added to another form.");
    }
    form->insert(control);
}

}  // namespace vm
