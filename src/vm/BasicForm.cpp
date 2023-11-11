#include "BasicForm.h"
#include "BasicFormsStorage.h"
#include "Error.h"

namespace vm {

/* static */ int64_t BasicForm::newForm(Interpreter* interpreter) {
    auto id = basicFormsStorage.nextId();
    basicFormsStorage.forms.add(id, new BasicForm(id, interpreter));
    return id;
}

BasicForm::BasicForm(int64_t id, Interpreter* interpreter)
    : TDialog(TRect(0, 0, 40, 15), "Form"), TWindowInit(initFrame), _id(id), _interpreter(interpreter) {
    options |= ofCentered;
    (void)_interpreter;  // temporary
}

BasicForm::~BasicForm() {
    basicFormsStorage.forms.remove(_id);
}

}  // namespace vm
