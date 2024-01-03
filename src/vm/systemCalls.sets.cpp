#include "vm/systemCall.h"
#include "vm/castObject.h"
#include "vm/Error.h"
#include "vm/Set.h"

namespace vm {

void systemCallObjectSetAdd(const SystemCallInput& input, SystemCallResult* result) {
    const auto& set = *castObjectSet(input.getObject(-2));
    const auto& key = input.getObjectPtr(-1);
    result->returnedObject = boost::make_local_shared<ObjectSet>(set, true, key);
}

void systemCallObjectSetBuilderAdd(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto* builder = castObjectSetBuilder(input.getObject(-2));
    const auto& key = input.getObjectPtr(-1);
    builder->keys.insert(key);
}

void systemCallObjectSetBuilderEnd(const SystemCallInput& input, SystemCallResult* result) {
    auto* builder = castObjectSetBuilder(input.getObject(-1));
    result->returnedObject = boost::make_local_shared<ObjectSet>(builder);
}

void systemCallObjectSetBuilderNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_local_shared<ObjectSetBuilder>();
}

void systemCallObjectSetNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_local_shared<ObjectSet>();
}

void systemCallObjectSetRemove(const SystemCallInput& input, SystemCallResult* result) {
    const auto& set = *castObjectSet(input.getObject(-2));
    const auto& key = input.getObjectPtr(-1);
    result->returnedObject = boost::make_local_shared<ObjectSet>(set, false, key);
}

void systemCallObjectSetUnion(const SystemCallInput& input, SystemCallResult* result) {
    const auto* lhs = castObjectSet(input.getObject(-2));
    const auto* rhs = castObjectSet(input.getObject(-1));
    result->returnedObject = lhs->unionWith(*rhs);
}

void systemCallObjectSetExcept(const SystemCallInput& input, SystemCallResult* result) {
    const auto* lhs = castObjectSet(input.getObject(-2));
    const auto* rhs = castObjectSet(input.getObject(-1));
    result->returnedObject = lhs->except(*rhs);
}

void systemCallSetContains(const SystemCallInput& input, SystemCallResult* result) {
    // Two possible signatures::
    // 2 objects - (ObjectList, Object)
    // 1 object, 1 value - (ValueList, Value)
    auto isValueSet = input.numValueArguments == 1;
    if (isValueSet) {
        const auto& set = *castValueSet(input.getObject(-1));
        const auto& key = input.getValue(-1);
        result->returnedValue = Value{ set.keys.find(key) != nullptr };
    } else {
        const auto& set = *castObjectSet(input.getObject(-2));
        const auto& key = input.getObjectPtr(-1);
        result->returnedValue = Value{ set.keys.find(key) != nullptr };
    }
}

void systemCallSetLen(const SystemCallInput& input, SystemCallResult* result) {
    const auto& setObject = input.getObject(-1);
    switch (setObject->getObjectType()) {
        case ObjectType::kValueSet:
            result->returnedValue = Value{ static_cast<uint64_t>(castValueSet(setObject)->keys.size()) };
            break;

        case ObjectType::kObjectSet:
            result->returnedValue = Value{ static_cast<uint64_t>(castObjectSet(setObject)->keys.size()) };
            break;

        default:
            throw Error(ErrorCode::kInternalTypeConfusion, "SetLen: Expected a ValueSet or ObjectSet.");
    }
}

void systemCallSetValues(const SystemCallInput& input, SystemCallResult* result) {
    const auto& setObject = input.getObject(-1);
    switch (setObject->getObjectType()) {
        case ObjectType::kValueSet:
            result->returnedObject = castValueSet(setObject)->toList();
            break;

        case ObjectType::kObjectSet:
            result->returnedObject = castObjectSet(setObject)->toList();
            break;

        default:
            throw Error(ErrorCode::kInternalTypeConfusion, "SetLen: Expected a ValueSet or ObjectSet.");
    }
}

void systemCallValueSetAdd(const SystemCallInput& input, SystemCallResult* result) {
    const auto& set = *castValueSet(input.getObject(-1));
    const auto& key = input.getValue(-1);
    result->returnedObject = boost::make_local_shared<ValueSet>(set, true, key);
}

void systemCallValueSetBuilderAdd(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto* builder = castValueSetBuilder(input.getObject(-1));
    const auto& key = input.getValue(-1);
    builder->keys.insert(key);
}

void systemCallValueSetBuilderEnd(const SystemCallInput& input, SystemCallResult* result) {
    auto* builder = castValueSetBuilder(input.getObject(-1));
    result->returnedObject = boost::make_local_shared<ValueSet>(builder);
}

void systemCallValueSetBuilderNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_local_shared<ValueSetBuilder>();
}

void systemCallValueSetNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_local_shared<ValueSet>();
}

void systemCallValueSetRemove(const SystemCallInput& input, SystemCallResult* result) {
    const auto& set = *castValueSet(input.getObject(-1));
    const auto& key = input.getValue(-1);
    result->returnedObject = boost::make_local_shared<ValueSet>(set, false, key);
}

void systemCallValueSetUnion(const SystemCallInput& input, SystemCallResult* result) {
    const auto* lhs = castValueSet(input.getObject(-2));
    const auto* rhs = castValueSet(input.getObject(-1));
    result->returnedObject = lhs->unionWith(*rhs);
}

void systemCallValueSetExcept(const SystemCallInput& input, SystemCallResult* result) {
    const auto* lhs = castValueSet(input.getObject(-2));
    const auto* rhs = castValueSet(input.getObject(-1));
    result->returnedObject = lhs->except(*rhs);
}

}  // namespace vm
