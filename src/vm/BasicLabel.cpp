#include "BasicLabel.h"
#include "BasicFormsStorage.h"

namespace vm {

/* static */ int64_t BasicLabel::newLabel(Interpreter* interpreter) {
    auto id = basicFormsStorage.nextId();
    basicFormsStorage.controls.add(id, new BasicLabel(id, interpreter));
    return id;
}

BasicLabel::BasicLabel(int64_t id, Interpreter* interpreter)
    : TLabel(TRect{ 1, 1, 15, 2 }, "", nullptr), _id(id), _interpreter(interpreter) {
    (void)_interpreter;  // temporary
}

BasicLabel::~BasicLabel() {
    basicFormsStorage.controls.remove(_id);
}

boost::intrusive_ptr<String> BasicLabel::getBasicText() {
    return boost::make_intrusive_ptr<String>(text);
}

void BasicLabel::setText(const String& newText) {
    auto str = newText.value;
    delete[] text;  // NOLINT(cppcoreguidelines-owning-memory)
    text = newStr(str.c_str());
}

}  // namespace vm
