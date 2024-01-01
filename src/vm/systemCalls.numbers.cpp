#include "systemCall.h"
#include "String.h"

namespace vm {

void systemCallAbs(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num.abs();
}

void systemCallAcos(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setDouble(std::acos(input.getValue(-1).getDouble()));
}

void systemCallAsin(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setDouble(std::asin(input.getValue(-1).getDouble()));
}

void systemCallAtan(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setDouble(std::atan(input.getValue(-1).getDouble()));
}

void systemCallAtan2(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setDouble(std::atan2(input.getValue(-2).getDouble(), input.getValue(-1).getDouble()));
}

void systemCallCeil(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num.ceil();
}

void systemCallCos(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setDouble(std::cos(input.getValue(-1).getDouble()));
}

void systemCallExp(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num.exp();
}

void systemCallFloor(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num.floor();
}

void systemCallLog(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num.ln();
}

void systemCallLog10(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num.log10();
}

void systemCallNumberAdd(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-2).num + input.getValue(-1).num;
}

void systemCallNumberDivide(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-2).num / input.getValue(-1).num;
}

void systemCallNumberEquals(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num =
        input.getValue(-2).num == input.getValue(-1).num ? util::kDecimalOne : util::kDecimalZero;
}

void systemCallNumberGreaterThan(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num =
        input.getValue(-2).num > input.getValue(-1).num ? util::kDecimalOne : util::kDecimalZero;
}

void systemCallNumberGreaterThanEquals(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num =
        input.getValue(-2).num >= input.getValue(-1).num ? util::kDecimalOne : util::kDecimalZero;
}

void systemCallNumberLessThan(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num =
        input.getValue(-2).num < input.getValue(-1).num ? util::kDecimalOne : util::kDecimalZero;
}

void systemCallNumberLessThanEquals(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num =
        input.getValue(-2).num <= input.getValue(-1).num ? util::kDecimalOne : util::kDecimalZero;
}

void systemCallNumberModulus(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-2).num % input.getValue(-1).num;
}

void systemCallNumberMultiply(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-2).num * input.getValue(-1).num;
}

void systemCallNumberNotEquals(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num =
        input.getValue(-2).num != input.getValue(-1).num ? util::kDecimalOne : util::kDecimalZero;
}

void systemCallNumberSubtract(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-2).num - input.getValue(-1).num;
}

void systemCallNumberToString(const SystemCallInput& input, SystemCallResult* result) {
    const auto& value = input.getValue(-1);
    result->returnedObject = boost::make_local_shared<String>(value.getString());
}

void systemCallPow(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-2).num.pow(input.getValue(-1).num);
}

void systemCallRound(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = util::round(input.getValue(-1).num);
}

void systemCallSin(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setDouble(std::sin(input.getValue(-1).getDouble()));
}

void systemCallSqr(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num.sqrt();
}

void systemCallTan(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setDouble(std::tan(input.getValue(-1).getDouble()));
}

void systemCallTrunc(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = input.getValue(-1).num.trunc();
}

}  // namespace vm
