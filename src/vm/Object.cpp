#include "Object.h"

namespace vm {

Object::Object() = default;

Object::~Object() = default;

bool operator==(const boost::local_shared_ptr<vm::Object>& lhs, const boost::local_shared_ptr<vm::Object>& rhs) {
    return lhs->equals(*rhs);
}

}  // namespace vm
