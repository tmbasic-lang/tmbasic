#include "Value.h"
#include "util/decimal.h"

namespace vm {

Value::Value() = default;

Value::Value(decimal::Decimal num) : num(std::move(num)) {}

bool Value::operator==(const Value& rhs) const {
    return num == rhs.num;
}

bool Value::getBoolean() const {
    return num != 0;
}

void Value::setBoolean(bool value) {
    num = value ? 1 : 0;
}

int32_t Value::getInt32() const {
    return num.floor().i32();
}

int64_t Value::getInt64() const {
    return num.floor().i64();
}

std::string Value::getString() const {
    return util::decimalToString(num);
}

std::size_t Value::getHash() const {
    return std::hash<int64_t>{}(num.floor().i64());
}

}  // namespace vm
