#include "Object.h"

using namespace vm;

Object::~Object() {}

bool operator==(const boost::local_shared_ptr<vm::Object>& lhs, const boost::local_shared_ptr<vm::Object>& rhs) {
    return lhs->equals(*rhs);
}
