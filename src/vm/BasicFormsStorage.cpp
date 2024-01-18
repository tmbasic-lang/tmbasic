#include "BasicFormsStorage.h"

namespace vm {

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
BasicFormsStorage basicFormsStorage{};

void BasicFormsStorage::clear() {
    forms.clear();
    controls.clear();
}

}  // namespace vm
