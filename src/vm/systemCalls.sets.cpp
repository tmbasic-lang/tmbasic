#include "vm/systemCall.h"
#include "vm/castObject.h"
#include "vm/Error.h"
#include "vm/Set.h"

namespace vm {

void initSystemCallsSets() {
    initSystemCall(SystemCall::kObjectSetAdd, [](const auto& input, auto* result) {
        const auto& set = *castObjectSet(input.getObject(-2));
        const auto& key = input.getObjectPtr(-1);
        result->returnedObject = boost::make_local_shared<ObjectSet>(set, true, key);
    });

    initSystemCall(SystemCall::kObjectSetBuilderAdd, [](const auto& input, auto* /*result*/) {
        auto* builder = castObjectSetBuilder(input.getObject(-2));
        const auto& key = input.getObjectPtr(-1);
        builder->keys.insert(key);
    });

    initSystemCall(SystemCall::kObjectSetBuilderEnd, [](const auto& input, auto* result) {
        auto* builder = castObjectSetBuilder(input.getObject(-1));
        result->returnedObject = boost::make_local_shared<ObjectSet>(builder);
    });

    initSystemCall(SystemCall::kObjectSetBuilderNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ObjectSetBuilder>();
    });

    initSystemCall(SystemCall::kObjectSetNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ObjectSet>();
    });

    initSystemCall(SystemCall::kObjectSetRemove, [](const auto& input, auto* result) {
        const auto& set = *castObjectSet(input.getObject(-2));
        const auto& key = input.getObjectPtr(-1);
        result->returnedObject = boost::make_local_shared<ObjectSet>(set, false, key);
    });

    initSystemCall(SystemCall::kObjectSetUnion, [](const auto& input, auto* result) {
        const auto& lhsObject = input.getObject(-2);
        const auto& rhsObject = input.getObject(-1);
        result->returnedObject =
            boost::make_local_shared<ObjectSet>(*castObjectSet(lhsObject), *castObjectSet(rhsObject));
    });

    initSystemCall(SystemCall::kSetContains, [](const auto& input, auto* result) {
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
    });

    initSystemCall(SystemCall::kSetLen, [](const auto& input, auto* result) {
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
    });

    initSystemCall(SystemCall::kSetValues, [](const auto& input, auto* result) {
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
    });

    initSystemCall(SystemCall::kValueSetAdd, [](const auto& input, auto* result) {
        const auto& set = *castValueSet(input.getObject(-1));
        const auto& key = input.getValue(-1);
        result->returnedObject = boost::make_local_shared<ValueSet>(set, true, key);
    });

    initSystemCall(SystemCall::kValueSetBuilderAdd, [](const auto& input, auto* /*result*/) {
        auto* builder = castValueSetBuilder(input.getObject(-1));
        const auto& key = input.getValue(-1);
        builder->keys.insert(key);
    });

    initSystemCall(SystemCall::kValueSetBuilderEnd, [](const auto& input, auto* result) {
        auto* builder = castValueSetBuilder(input.getObject(-1));
        result->returnedObject = boost::make_local_shared<ValueSet>(builder);
    });

    initSystemCall(SystemCall::kValueSetBuilderNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ValueSetBuilder>();
    });

    initSystemCall(SystemCall::kValueSetNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ValueSet>();
    });

    initSystemCall(SystemCall::kValueSetRemove, [](const auto& input, auto* result) {
        const auto& set = *castValueSet(input.getObject(-1));
        const auto& key = input.getValue(-1);
        result->returnedObject = boost::make_local_shared<ValueSet>(set, false, key);
    });

    initSystemCall(SystemCall::kValueSetUnion, [](const auto& input, auto* result) {
        const auto& lhsObject = input.getObject(-2);
        const auto& rhsObject = input.getObject(-1);
        result->returnedObject = boost::make_local_shared<ValueSet>(*castValueSet(lhsObject), *castValueSet(rhsObject));
    });
}

}  // namespace vm
