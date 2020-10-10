#include "Value.h"

using namespace vm;

bool Value::operator==(const Value& rhs) const {
    return num == rhs.num;
}

bool Value::getBoolean() const {
    return num != 0;
}

void Value::setBoolean(bool value) {
    num = value ? 1 : 0;
}

double Value::getDouble() const {
    return std::decimal::decimal64_to_double(num);
}

int64_t Value::getInt64() const {
    return std::decimal::decimal64_to_long_long(num);
}
