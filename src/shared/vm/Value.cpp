#include "Value.h"

namespace vm {

bool Value::operator==(const Value& rhs) const {
    return num == rhs.num;
}

bool Value::getBoolean() const {
    return num != 0;
}

void Value::setBoolean(bool value) {
    num = value ? 1 : 0;
}

int64_t Value::getInt64() const {
    return num.floor().i64();
}

}  // namespace vm
