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

static bool _systemCallsInitialized = false;
static std::vector<SystemCallFunc> _systemCalls;

SystemCallInput::SystemCallInput(
    std::array<Value, kValueStackSize>* valueStack,
    std::array<boost::local_shared_ptr<Object>, kObjectStackSize>* objectStack,
    int valueStackIndex,
    int objectStackIndex,
    std::istream* consoleInputStream,
    std::ostream* consoleOutputStream,
    const Value& errorCode,
    const std::string& errorMessage)
    : valueStack(valueStack),
      objectStack(objectStack),
      valueStackIndex(valueStackIndex),
      objectStackIndex(objectStackIndex),
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

// Defined in systemCalls.*.cpp
void initSystemCallsDates();
void initSystemCallsFiles();
void initSystemCallsLists();
void initSystemCallsMaps();
void initSystemCallsNumbers();
void initSystemCallsOptionals();
void initSystemCallsStrings();

void initSystemCalls() {
    if (_systemCallsInitialized) {
        return;
    }

    initSystemCallsDates();
    initSystemCallsFiles();
    initSystemCallsLists();
    initSystemCallsMaps();
    initSystemCallsNumbers();
    initSystemCallsOptionals();
    initSystemCallsStrings();

    initSystemCall(SystemCall::kBooleanAnd, [](const auto& input, auto* result) {
        result->returnedValue.setBoolean(input.getValue(-2).getBoolean() && input.getValue(-1).getBoolean());
    });

    initSystemCall(SystemCall::kBooleanNot, [](const auto& input, auto* result) {
        result->returnedValue.setBoolean(!input.getValue(-1).getBoolean());
    });

    initSystemCall(SystemCall::kBooleanOr, [](const auto& input, auto* result) {
        result->returnedValue.setBoolean(input.getValue(-2).getBoolean() || input.getValue(-1).getBoolean());
    });

    initSystemCall(SystemCall::kCounterIsPastLimit, [](const auto& input, auto* result) {
        // used with 'for' loops
        const auto& counter = input.getValue(-3).num;
        const auto& limit = input.getValue(-2).num;
        const auto& step = input.getValue(-1).num;
        bool condition{};
        if (step >= 0) {
            condition = counter > limit;
        } else {
            condition = counter < limit;
        }
        result->returnedValue.setBoolean(condition);
    });

    initSystemCall(
        SystemCall::kErrorCode, [](const auto& input, auto* result) { result->returnedValue = input.errorCode; });

    initSystemCall(SystemCall::kErrorMessage, [](const auto& input, auto* result) {
        result->returnedObject = boost::make_local_shared<String>(input.errorMessage);
    });

    initSystemCall(SystemCall::kFlushConsoleOutput, [](const auto& input, auto* /*result*/) {
        input.consoleOutputStream->flush();
    });

    initSystemCall(SystemCall::kObjectEquals, [](const auto& input, auto* result) {
        result->returnedValue.num = input.getObject(-2).equals(input.getObject(-1)) ? 1 : 0;
    });

    _systemCallsInitialized = true;
}

SystemCallResult systemCall(SystemCall which, const SystemCallInput& input) {
    SystemCallResult result;

    try {
        auto index = static_cast<size_t>(which);
        _systemCalls.at(index)(input, &result);
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
