#include "systemCall.h"
#include "Error.h"
#include "List.h"
#include "Map.h"
#include "Optional.h"
#include "String.h"
#include "TimeZone.h"
#include "constants.h"
#include "date.h"
#include "filesystem.h"
#include "util/decimal.h"
#include "util/path.h"

namespace vm {

static std::vector<SystemCallFunc> _systemCalls;

SystemCallInput::SystemCallInput(
    Interpreter* interpreter,
    std::array<Value, kValueStackSize>* valueStack,
    std::array<boost::local_shared_ptr<Object>, kObjectStackSize>* objectStack,
    int valueStackIndex,
    int objectStackIndex,
    int numValueArguments,
    int numObjectArguments,
    std::istream* consoleInputStream,
    std::ostream* consoleOutputStream,
    const Value& errorCode,
    const std::string& errorMessage)
    : interpreter(interpreter),
      valueStack(valueStack),
      objectStack(objectStack),
      valueStackIndex(valueStackIndex),
      objectStackIndex(objectStackIndex),
      numValueArguments(numValueArguments),
      numObjectArguments(numObjectArguments),
      consoleInputStream(consoleInputStream),
      consoleOutputStream(consoleOutputStream),
      errorCode(errorCode),
      errorMessage(errorMessage) {}

void initSystemCall(SystemCall which, SystemCallFunc func) {
    auto index = static_cast<size_t>(which);

    while (_systemCalls.size() <= index) {
        _systemCalls.push_back(nullptr);
    }

    _systemCalls.at(index) = func;
}

void systemCallBooleanAnd(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setBoolean(input.getValue(-2).getBoolean() && input.getValue(-1).getBoolean());
}

void systemCallBooleanNot(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setBoolean(!input.getValue(-1).getBoolean());
}

void systemCallBooleanOr(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.setBoolean(input.getValue(-2).getBoolean() || input.getValue(-1).getBoolean());
}

void systemCallCounterIsPastLimit(const SystemCallInput& input, SystemCallResult* result) {
    // used with 'for' loops
    const auto& counter = input.getValue(-3).num;
    const auto& limit = input.getValue(-2).num;
    const auto& step = input.getValue(-1).num;
    bool condition{};
    if (step.sign() > 0 || step.iszero()) {
        condition = counter > limit;
    } else {
        condition = counter < limit;
    }
    result->returnedValue.setBoolean(condition);
}

void systemCallErrorCode(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue = input.errorCode;
}

void systemCallErrorMessage(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedObject = boost::make_local_shared<String>(input.errorMessage);
}

void systemCallFlushConsoleOutput(const SystemCallInput& input, SystemCallResult* /*result*/) {
    input.consoleOutputStream->flush();
}

void systemCallObjectEquals(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num =
        input.getObject(-2)->equals(*input.getObject(-1)) ? util::kDecimalOne : util::kDecimalZero;
}

SystemCallResult systemCall(SystemCall which, const SystemCallInput& input) {
    SystemCallResult result;

    auto index = static_cast<size_t>(which);
#ifdef NDEBUG
    auto& func = _systemCalls[index];
#else
    auto& func = _systemCalls.at(index);
    if (func == nullptr) {
        throw std::runtime_error(fmt::format("System call not implemented: {}", NAMEOF_ENUM(which)));
    }
#endif

    try {
        func(input, &result);
    } catch (Error& ex) {
        result.hasError = true;
        result.errorCode = static_cast<int>(ex.code);
        result.errorMessage = ex.what();
    } catch (std::exception& ex) {
        result.hasError = true;
        result.errorCode = -1;
        result.errorMessage = ex.what();
    }

    return result;
}

std::optional<SystemCall> getDualGenericSystemCall(SystemCall call) {
    switch (call) {
        case SystemCall::kListFillO:
            return SystemCall::kListFillV;
        default:
            return {};
    }
}

}  // namespace vm
