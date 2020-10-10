#pragma once

#include "common.h"

namespace vm {

typedef std::decimal::decimal64 FloatValue;
typedef int64_t IntValue;

struct Value {
    FloatValue num;
    bool getBoolean() const;
    void setBoolean(bool value);
    double getDouble() const;
    int64_t getInt64() const;
    bool operator==(const Value& rhs) const;
};

struct ValuePointerCompare {
    bool operator()(const Value& lhs, const Value*& rhs) { return lhs == *rhs; }
};

}  // namespace vm

namespace std {

template <>
struct hash<vm::Value> {
    std::size_t operator()(vm::Value const& k) const noexcept {
        return hash<double>{}(std::decimal::decimal64_to_double(k.num));
    }
};

}  // namespace std
