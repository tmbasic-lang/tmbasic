#include "Value.h"
#include "util/decimal.h"

namespace vm {

Value::Value() = default;

Value::Value(decimal::Decimal num) : num(std::move(num)) {}

bool Value::operator==(const Value& rhs) const {
    return num == rhs.num;
}

std::string Value::getString() const {
    return util::decimalToString(num);
}

std::size_t Value::getHash() const {
    return std::hash<int64_t>{}(num.floor().i64());
}

}  // namespace vm
