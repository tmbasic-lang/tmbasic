#include "systemCall.h"
#include "List.h"
#include "String.h"

namespace vm {

SystemCallInput::SystemCallInput(
    const std::array<Value, kValueStackSize>& valueStack,
    const std::array<boost::local_shared_ptr<Object>, kObjectStackSize>& objectStack,
    int valueStackIndex,
    int objectStackIndex)
    : valueStack(valueStack),
      objectStack(objectStack),
      valueStackIndex(valueStackIndex),
      objectStackIndex(objectStackIndex) {}

SystemCallResult::SystemCallResult(int popValues, int popObjects) : popValues(popValues), popObjects(popObjects) {}

SystemCallResult::SystemCallResult(Value a, int popValues, int popObjects)
    : a(std::move(a)), popValues(popValues), popObjects(popObjects) {}

SystemCallResult::SystemCallResult(boost::local_shared_ptr<Object> x, int popValues, int popObjects)
    : x(std::move(x)), popValues(popValues), popObjects(popObjects) {}

static SystemCallResult systemCallAvailableLocales() {
    // no parameters. returns ObjectList of String in X
    int32_t count = 0;
    const auto* locales = icu::Locale::getAvailableLocales(count);

    auto objectListBuilder = ObjectListBuilder();
    for (int32_t i = 0; i < count; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto* name = locales[i].getName();
        objectListBuilder.items.push_back(boost::make_local_shared<String>(name, strlen(name)));
    }

    return SystemCallResult(boost::make_local_shared<ObjectList>(&objectListBuilder), 0, 0);
}

static SystemCallResult systemCallChr(const SystemCallInput& input) {
    auto value = input.valueStack.at(input.valueStackIndex).getInt64();
    auto ch = static_cast<UChar32>(value);
    return SystemCallResult(
        boost::make_local_shared<String>(ch > 0 ? icu::UnicodeString(ch) : icu::UnicodeString()), 1, 0);
}

SystemCallResult systemCall(SystemCall which, const SystemCallInput& input) {
    switch (which) {
        case SystemCall::kAvailableLocales:
            return systemCallAvailableLocales();
        case SystemCall::kChr:
            return systemCallChr(input);
        default:
            assert(false);
            return SystemCallResult(0, 0);
    }
}

}  // namespace vm
