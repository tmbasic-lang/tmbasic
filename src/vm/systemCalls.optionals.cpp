#include "systemCall.h"
#include "vm/Error.h"
#include "vm/Optional.h"
#include "vm/castObject.h"

namespace vm {

static std::pair<const ValueOptional*, const ObjectOptional*> valueOrObjectOptional(const Object& object) {
    auto type = object.getObjectType();

    if (type == ObjectType::kValueOptional) {
        return { &castValueOptional(object), nullptr };
    }

    if (type == ObjectType::kObjectOptional) {
        return { nullptr, &castObjectOptional(object) };
    }

    throw Error(
        ErrorCode::kInternalTypeConfusion,
        fmt::format(
            "Internal type confusion error. Target is neither {} nor {}. Actual: {}", NAMEOF_TYPE(ValueOptional),
            NAMEOF_TYPE(ObjectOptional), NAMEOF_ENUM(type)));
}

void initSystemCallsOptionals() {
    initSystemCall(SystemCall::kHasValue, [](const auto& input, auto* result) {
        const auto valueOrObject = valueOrObjectOptional(input.getObject(-1));
        const auto* valueOptional = valueOrObject.first;
        const auto* objectOptional = valueOrObject.second;

        result->returnedValue.setBoolean(
            valueOptional != nullptr ? valueOptional->item.has_value() : objectOptional->item.has_value());
    });

    initSystemCall(SystemCall::kObjectOptionalNewMissing, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ObjectOptional>();
    });

    initSystemCall(SystemCall::kObjectOptionalNewPresent, [](const auto& input, auto* result) {
        result->returnedObject = boost::make_local_shared<ObjectOptional>(input.getObjectPtr(-1));
    });

    initSystemCall(SystemCall::kValueOptionalNewMissing, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ValueOptional>();
    });

    initSystemCall(SystemCall::kValueOptionalNewPresent, [](const auto& input, auto* result) {
        result->returnedObject = boost::make_local_shared<ValueOptional>(input.getValue(-1));
    });

    initSystemCall(SystemCall::kValue, [](const auto& input, auto* result) {
        const auto valueOrObject = valueOrObjectOptional(input.getObject(-1));
        const auto* valueOptional = valueOrObject.first;
        const auto* objectOptional = valueOrObject.second;

        if (valueOptional != nullptr) {
            if (!valueOptional->item.has_value()) {
                throw Error(ErrorCode::kValueNotPresent, "Optional value is not present.");
            }
            result->returnedValue = *valueOptional->item;
        } else {
            if (!objectOptional->item.has_value()) {
                throw Error(ErrorCode::kValueNotPresent, "Optional value is not present.");
            }
            result->returnedObject = *objectOptional->item;
        }
    });
}

}  // namespace vm
