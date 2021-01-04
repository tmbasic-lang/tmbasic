#pragma once

#include "../../common.h"

namespace vm {

struct Value {
    Value();
    explicit Value(decimal::Decimal num);
    decimal::Decimal num;
    std::string getString() const;
    bool getBoolean() const;
    void setBoolean(bool value);
    int64_t getInt64() const;
    bool operator==(const Value& rhs) const;
};

struct ValuePointerCompare {
    bool operator()(const Value& lhs, const Value* rhs) { return lhs == *rhs; }
};

}  // namespace vm

namespace std {

template <>
struct hash<vm::Value> {
    std::size_t operator()(vm::Value const& k) const noexcept { return hash<int64_t>{}(k.num.floor().i64()); }
};

}  // namespace std
