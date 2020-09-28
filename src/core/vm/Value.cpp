#include "Value.h"

using namespace vm;

bool Value::operator==(const Value& rhs) const {
    return bits == rhs.bits;
}
