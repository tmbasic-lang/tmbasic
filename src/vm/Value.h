#pragma once

#include "../common.h"
#include "../shared/decimal.h"

namespace vm {

struct Value {
    decimal::Decimal num;
    inline Value() {}
    inline explicit Value(decimal::Decimal num) : num(std::move(num)) {}
    inline explicit Value(double num) : num(shared::doubleToDecimal(num)) {}
    inline explicit Value(int32_t num) : num(num) {}
    inline explicit Value(int64_t num) : num(num) {}
    inline explicit Value(uint32_t num) : num(num) {}
    inline explicit Value(uint64_t num) : num(num) {}
    inline explicit Value(bool x) : num(x ? shared::kDecimalOne : shared::kDecimalZero) {}
    inline std::string getString() const { return shared::decimalToString(num); }
    inline bool getBoolean() const { return !num.iszero(); }
    inline void setBoolean(bool value) { num = value ? shared::kDecimalOne : shared::kDecimalZero; }
    inline int32_t getInt32() const { return num.floor().i32(); }
    inline int64_t getInt64() const { return num.floor().i64(); }
    inline uint32_t getUint32() const { return num.floor().u32(); }
    inline uint64_t getUint64() const { return num.floor().u64(); }
    inline double getDouble() const { return shared::decimalToDouble(num); }
    inline void setDouble(double value) { num = shared::doubleToDecimal(value); }
    inline bool operator==(const Value& rhs) const { return num == rhs.num; }
    inline std::size_t getHash() const { return shared::getDecimalHash(num); }
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
