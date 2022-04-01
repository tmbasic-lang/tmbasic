#include "systemCall.h"
#include "String.h"

namespace vm {

void initSystemCallsNumbers() {
    initSystemCall(SystemCall::kAbs, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num.abs();
    });

    initSystemCall(SystemCall::kAcos, [](const auto& input, auto* result) {
        result->returnedValue.setDouble(std::acos(input.getValue(-1).getDouble()));
    });

    initSystemCall(SystemCall::kAsin, [](const auto& input, auto* result) {
        result->returnedValue.setDouble(std::asin(input.getValue(-1).getDouble()));
    });

    initSystemCall(SystemCall::kAtan, [](const auto& input, auto* result) {
        result->returnedValue.setDouble(std::atan(input.getValue(-1).getDouble()));
    });

    initSystemCall(SystemCall::kAtan2, [](const auto& input, auto* result) {
        result->returnedValue.setDouble(std::atan2(input.getValue(-2).getDouble(), input.getValue(-1).getDouble()));
    });

    initSystemCall(SystemCall::kCeil, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num.ceil();
    });

    initSystemCall(SystemCall::kCos, [](const auto& input, auto* result) {
        result->returnedValue.setDouble(std::cos(input.getValue(-1).getDouble()));
    });

    initSystemCall(SystemCall::kExp, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num.exp();
    });

    initSystemCall(SystemCall::kFloor, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num.floor();
    });

    initSystemCall(SystemCall::kLog, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num.ln();
    });

    initSystemCall(SystemCall::kLog10, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num.log10();
    });

    initSystemCall(SystemCall::kNumberAdd, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num + input.getValue(-1).num;
    });

    initSystemCall(SystemCall::kNumberDivide, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num / input.getValue(-1).num;
    });

    initSystemCall(SystemCall::kNumberEquals, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num == input.getValue(-1).num ? 1 : 0;
    });

    initSystemCall(SystemCall::kNumberGreaterThan, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num > input.getValue(-1).num ? 1 : 0;
    });

    initSystemCall(SystemCall::kNumberGreaterThanEquals, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num >= input.getValue(-1).num ? 1 : 0;
    });

    initSystemCall(SystemCall::kNumberLessThan, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num < input.getValue(-1).num ? 1 : 0;
    });

    initSystemCall(SystemCall::kNumberLessThanEquals, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num <= input.getValue(-1).num ? 1 : 0;
    });

    initSystemCall(SystemCall::kNumberModulus, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num % input.getValue(-1).num;
    });

    initSystemCall(SystemCall::kNumberMultiply, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num * input.getValue(-1).num;
    });

    initSystemCall(SystemCall::kNumberNotEquals, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num != input.getValue(-1).num ? 1 : 0;
    });

    initSystemCall(SystemCall::kNumberSubtract, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num - input.getValue(-1).num;
    });

    initSystemCall(SystemCall::kNumberToString, [](const auto& input, auto* result) {
        const auto& value = input.getValue(-1);
        result->returnedObject = boost::make_local_shared<String>(value.getString());
    });

    initSystemCall(SystemCall::kPow, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-2).num.pow(input.getValue(-1).num);
    });

    initSystemCall(SystemCall::kRound, [](const auto& input, auto* result) {
        result->returnedValue.num = util::round(input.getValue(-1).num);
    });

    initSystemCall(SystemCall::kSin, [](const auto& input, auto* result) {
        result->returnedValue.setDouble(std::sin(input.getValue(-1).getDouble()));
    });

    initSystemCall(SystemCall::kSqr, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num.sqrt();
    });

    initSystemCall(SystemCall::kTan, [](const auto& input, auto* result) {
        result->returnedValue.setDouble(std::tan(input.getValue(-1).getDouble()));
    });

    initSystemCall(SystemCall::kTrunc, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getValue(-1).num.trunc();
    });
}

}  // namespace vm
