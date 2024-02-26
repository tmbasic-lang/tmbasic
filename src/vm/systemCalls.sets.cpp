#include "vm/systemCall.h"
#include "shared/Error.h"
#include "vm/castObject.h"
#include "vm/Set.h"

using shared::Error;
using shared::ErrorCode;

namespace vm {

// (lhs as ObjectSet, rhs as Object) as ObjectSet
void systemCallObjectSetAdd(const SystemCallInput& input, SystemCallResult* result) {
    const auto& set = *castObjectSet(input.getObject(-2));
    const auto& key = input.getObjectPtr(-1);
    result->returnedObject = boost::make_intrusive_ptr<ObjectSet>(set, true, key);
}

// (builder as ObjectSetBuilder, Object as Object)
void systemCallObjectSetBuilderAdd(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto* builder = castObjectSetBuilder(input.getObject(-2));
    const auto& key = input.getObjectPtr(-1);
    builder->keys.insert(key);
}

// (builder as ObjectSetBuilder) as ObjectSet
void systemCallObjectSetBuilderEnd(const SystemCallInput& input, SystemCallResult* result) {
    auto* builder = castObjectSetBuilder(input.getObject(-1));
    result->returnedObject = boost::make_intrusive_ptr<ObjectSet>(builder);
}

// () as ObjectSetBuilder
void systemCallObjectSetBuilderNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_intrusive_ptr<ObjectSetBuilder>();
}

// () as ObjectSet
void systemCallObjectSetNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_intrusive_ptr<ObjectSet>();
}

// (lhs as ObjectSet, rhs as Object) as ObjectSet
void systemCallObjectSetRemove(const SystemCallInput& input, SystemCallResult* result) {
    const auto& set = *castObjectSet(input.getObject(-2));
    const auto& key = input.getObjectPtr(-1);
    result->returnedObject = boost::make_intrusive_ptr<ObjectSet>(set, false, key);
}

// (lhs as Set of T, rhs as Set of T) as Set of T
void systemCallObjectSetUnion(const SystemCallInput& input, SystemCallResult* result) {
    const auto* lhs = castObjectSet(input.getObject(-2));
    const auto* rhs = castObjectSet(input.getObject(-1));
    result->returnedObject = lhs->unionWith(*rhs);
}

// (lhs as ObjectSet, rhs as ObjectSet) as ObjectSet
void systemCallObjectSetExcept(const SystemCallInput& input, SystemCallResult* result) {
    const auto* lhs = castObjectSet(input.getObject(-2));
    const auto* rhs = castObjectSet(input.getObject(-1));
    result->returnedObject = lhs->except(*rhs);
}

// (input as Set of T, value as T) as Boolean
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

// (input as Set of T) as Number
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

// (input as Set of T) as List of T
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

// (lhs as ValueSet, rhs as Value) as ValueSet
void systemCallValueSetAdd(const SystemCallInput& input, SystemCallResult* result) {
    const auto& set = *castValueSet(input.getObject(-1));
    const auto& key = input.getValue(-1);
    result->returnedObject = boost::make_intrusive_ptr<ValueSet>(set, true, key);
}

// (builder as ValueSetBuilder, value as Value)
void systemCallValueSetBuilderAdd(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto* builder = castValueSetBuilder(input.getObject(-1));
    const auto& key = input.getValue(-1);
    builder->keys.insert(key);
}

// (builder as ValueSetBuilder) as ValueSet
void systemCallValueSetBuilderEnd(const SystemCallInput& input, SystemCallResult* result) {
    auto* builder = castValueSetBuilder(input.getObject(-1));
    result->returnedObject = boost::make_intrusive_ptr<ValueSet>(builder);
}

// () as ValueSetBuilder
void systemCallValueSetBuilderNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_intrusive_ptr<ValueSetBuilder>();
}

// () as ValueSet
void systemCallValueSetNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_intrusive_ptr<ValueSet>();
}

// (lhs as ValueSet, rhs as Value) as ValueSet
void systemCallValueSetRemove(const SystemCallInput& input, SystemCallResult* result) {
    const auto& set = *castValueSet(input.getObject(-1));
    const auto& key = input.getValue(-1);
    result->returnedObject = boost::make_intrusive_ptr<ValueSet>(set, false, key);
}

// (lhs as Set of T, rhs as Set of T) as Set of T
void systemCallValueSetUnion(const SystemCallInput& input, SystemCallResult* result) {
    const auto* lhs = castValueSet(input.getObject(-2));
    const auto* rhs = castValueSet(input.getObject(-1));
    result->returnedObject = lhs->unionWith(*rhs);
}

// (lhs as ValueSet, rhs as ValueSet) as ValueSet
void systemCallValueSetExcept(const SystemCallInput& input, SystemCallResult* result) {
    const auto* lhs = castValueSet(input.getObject(-2));
    const auto* rhs = castValueSet(input.getObject(-1));
    result->returnedObject = lhs->except(*rhs);
}

}  // namespace vm
