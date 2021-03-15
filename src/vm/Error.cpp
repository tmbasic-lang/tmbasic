#include "Error.h"

namespace vm {

Error::Error(ErrorCode code, const char* message) : code(code), std::runtime_error(message) {}

}  // namespace vm
