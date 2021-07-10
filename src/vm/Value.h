#pragma once

#include "../common.h"

namespace vm {

struct Value {
    decimal::Decimal num;
    Value();
    explicit Value(decimal::Decimal num);
    std::string getString() const;
    inline bool getBoolean() const { return num != 0; }
    inline void setBoolean(bool value) { num = value ? 1 : 0; }
    inline int32_t getInt32() const { return num.floor().i32(); }
    inline int64_t getInt64() const { return num.floor().i64(); }
    bool operator==(const Value& rhs) const;
    std::size_t getHash() const;
};

struct ValuePointerCompare {
    bool operator()(const Value& lhs, const Value* rhs) { return lhs == *rhs; }
};

}  // namespace vm

namespace std {

template <>
struct hash<vm::Value> {
    std::size_t operator()(vm::Value const& k) const noexcept { return k.getHash(); }
};

}  // namespace std
