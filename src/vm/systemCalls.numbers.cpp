#include "vm/systemCall.h"
#include "String.h"

namespace vm {

// (x as Number) as Number
void systemCallAbs(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num.abs();
}

// (x as Number) as Number
void systemCallAcos(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setDouble(std::acos(input.getValue(-1).getDouble()));
}

// (x as Number) as Number
void systemCallAsin(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setDouble(std::asin(input.getValue(-1).getDouble()));
}

// (x as Number) as Number
void systemCallAtan(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setDouble(std::atan(input.getValue(-1).getDouble()));
}

// (y as Number, x as Number) as Number
void systemCallAtan2(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setDouble(std::atan2(input.getValue(-2).getDouble(), input.getValue(-1).getDouble()));
}

// (x as Number) as Number
void systemCallCeil(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num.ceil();
}

// (x as Number) as Number
void systemCallCos(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setDouble(std::cos(input.getValue(-1).getDouble()));
}

// (x as Number) as Number
void systemCallExp(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num.exp();
}

// (x as Number) as Number
void systemCallFloor(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num.floor();
}

// (x as Number) as Number
void systemCallLog(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num.ln();
}

// (x as Number) as Number
void systemCallLog10(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num.log10();
}

// (lhs as Number, rhs as Number) as Number
void systemCallNumberAdd(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-2).num + input.getValue(-1).num;
}

// (lhs as Number, rhs as Number) as Number
void systemCallNumberDivide(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-2).num / input.getValue(-1).num;
}

// (lhs as Number, rhs as Number) as Boolean
void systemCallNumberEquals(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num =
        input.getValue(-2).num == input.getValue(-1).num ? shared::kDecimalOne : shared::kDecimalZero;
}

// (lhs as Number, rhs as Number) as Boolean
void systemCallNumberGreaterThan(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num =
        input.getValue(-2).num > input.getValue(-1).num ? shared::kDecimalOne : shared::kDecimalZero;
}

// (lhs as Number, rhs as Number) as Boolean
void systemCallNumberGreaterThanEquals(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num =
        input.getValue(-2).num >= input.getValue(-1).num ? shared::kDecimalOne : shared::kDecimalZero;
}

// (lhs as Number, rhs as Number) as Boolean
void systemCallNumberLessThan(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num =
        input.getValue(-2).num < input.getValue(-1).num ? shared::kDecimalOne : shared::kDecimalZero;
}

// (lhs as Number, rhs as Number) as Boolean
void systemCallNumberLessThanEquals(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num =
        input.getValue(-2).num <= input.getValue(-1).num ? shared::kDecimalOne : shared::kDecimalZero;
}

// (lhs as Number, rhs as Number) as Number
void systemCallNumberModulus(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-2).num % input.getValue(-1).num;
}

// (lhs as Number, rhs as Number) as Number
void systemCallNumberMultiply(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-2).num * input.getValue(-1).num;
}

// (lhs as Number, rhs as Number) as Boolean
void systemCallNumberNotEquals(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num =
        input.getValue(-2).num != input.getValue(-1).num ? shared::kDecimalOne : shared::kDecimalZero;
}

// (lhs as Number, rhs as Number) as Number
void systemCallNumberSubtract(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-2).num - input.getValue(-1).num;
}

// (input as Number) as String
void systemCallNumberToString(const SystemCallInput& input, SystemCallResult* result) {
    const auto& value = input.getValue(-1);
    result->returnedObject = boost::make_intrusive_ptr<String>(value.getString());
}

// (base as Number, exponent as Number) as Number
void systemCallPow(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-2).num.pow(input.getValue(-1).num);
}

// (x as Number) as Number
void systemCallRound(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = shared::round(input.getValue(-1).num);
}

// (x as Number) as Number
void systemCallSin(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setDouble(std::sin(input.getValue(-1).getDouble()));
}

// (x as Number) as Number
void systemCallSqr(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num.sqrt();
}

// (x as Number) as Number
void systemCallTan(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setDouble(std::tan(input.getValue(-1).getDouble()));
}

// (x as Number) as Number
void systemCallTrunc(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num.trunc();
}

// (x as Number, y as Number) as Number
void systemCallMin(const SystemCallInput& input, SystemCallResult* result) {
    auto x = input.getValue(-2).num;
    auto y = input.getValue(-1).num;
    result->returnedValue.num = x < y ? x : y;
}

// (x as Number, y as Number) as Number
void systemCallMax(const SystemCallInput& input, SystemCallResult* result) {
    auto x = input.getValue(-2).num;
    auto y = input.getValue(-1).num;
    result->returnedValue.num = x > y ? x : y;
}

}  // namespace vm
