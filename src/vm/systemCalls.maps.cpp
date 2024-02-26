#include "vm/systemCall.h"
#include "shared/Error.h"
#include "vm/castObject.h"
#include "vm/Map.h"

using shared::Error;
using shared::ErrorCode;

namespace vm {

// () as ObjectToObjectMap
void systemCallObjectToObjectMapNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_intrusive_ptr<ObjectToObjectMap>();
}

// (input as ObjectToObjectMap, key as Object, value as Object) as ObjectToObjectMap
void systemCallObjectToObjectMapSet(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map = *castObjectToObjectMap(input.getObject(-3));
    const auto& from = input.getObjectPtr(-2);
    const auto& to = input.getObjectPtr(-1);
    result->returnedObject = boost::make_intrusive_ptr<ObjectToObjectMap>(map, from, to);
}

// () as ObjectToObjectMapBuilder
void systemCallObjectToObjectMapBuilderNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_intrusive_ptr<ObjectToObjectMapBuilder>();
}

// (builder as ObjectToObjectMapBuilder, key as Object, value as Object)
void systemCallObjectToObjectMapBuilderAdd(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto& builder = *castObjectToObjectMapBuilder(input.getObject(-3));
    const auto& from = input.getObjectPtr(-2);
    const auto& to = input.getObjectPtr(-1);
    builder.pairs.set(from, to);
}

// (builder as ObjectToObjectMapBuilder) as ObjectToObjectMap
void systemCallObjectToObjectMapBuilderEnd(const SystemCallInput& input, SystemCallResult* result) {
    auto& builder = *castObjectToObjectMapBuilder(input.getObject(-1));
    result->returnedObject = boost::make_intrusive_ptr<ObjectToObjectMap>(&builder);
}

// () as ObjectToValueMap
void systemCallObjectToValueMapNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_intrusive_ptr<ObjectToValueMap>();
}

// (input as ObjectToValueMap, key as Object, value as Value) as ObjectToValueMap
void systemCallObjectToValueMapSet(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map = *castObjectToValueMap(input.getObject(-2));
    const auto& from = input.getObjectPtr(-1);
    const auto& to = input.getValue(-1);
    result->returnedObject = boost::make_intrusive_ptr<ObjectToValueMap>(map, from, to);
}

// () as ObjectToValueMapBuilder
void systemCallObjectToValueMapBuilderNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_intrusive_ptr<ObjectToValueMapBuilder>();
}

// (builder as ObjectToValueMapBuilder, key as Object, value as Value)
void systemCallObjectToValueMapBuilderAdd(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto& builder = *castObjectToValueMapBuilder(input.getObject(-2));
    const auto& from = input.getObjectPtr(-1);
    const auto& to = input.getValue(-1);
    builder.pairs.set(from, to);
}

// (builder as ObjectToValueMapBuilder) as ObjectToValueMap
void systemCallObjectToValueMapBuilderEnd(const SystemCallInput& input, SystemCallResult* result) {
    auto& builder = *castObjectToValueMapBuilder(input.getObject(-1));
    result->returnedObject = boost::make_intrusive_ptr<ObjectToValueMap>(&builder);
}

// () as ValueToObjectMap
void systemCallValueToObjectMapNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_intrusive_ptr<ValueToObjectMap>();
}

// (input as ValueToObjectMap, key as Value, value as Object) as ValueToObjectMap
void systemCallValueToObjectMapSet(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map = *castValueToObjectMap(input.getObject(-2));
    const auto& from = input.getValue(-1);
    const auto& to = input.getObjectPtr(-1);
    result->returnedObject = boost::make_intrusive_ptr<ValueToObjectMap>(map, from, to);
}

// () as ValueToObjectMapBuilder
void systemCallValueToObjectMapBuilderNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_intrusive_ptr<ValueToObjectMapBuilder>();
}

// (builder as ValueToObjectMapBuilder, key as Value, value as Object)
void systemCallValueToObjectMapBuilderAdd(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto& builder = *castValueToObjectMapBuilder(input.getObject(-2));
    const auto& from = input.getValue(-1);
    const auto& to = input.getObjectPtr(-1);
    builder.pairs.set(from, to);
}

// (builder as ValueToObjectMapBuilder) as ValueToObjectMap
void systemCallValueToObjectMapBuilderEnd(const SystemCallInput& input, SystemCallResult* result) {
    auto& builder = *castValueToObjectMapBuilder(input.getObject(-1));
    result->returnedObject = boost::make_intrusive_ptr<ValueToObjectMap>(&builder);
}

// () as ValueToValueMap
void systemCallValueToValueMapNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_intrusive_ptr<ValueToValueMap>();
}

// (input as ValueToValueMap, key as Value, value as Value) as ValueToValueMap
void systemCallValueToValueMapSet(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map = *castValueToValueMap(input.getObject(-1));
    const auto& from = input.getValue(-2);
    const auto& to = input.getValue(-1);
    result->returnedObject = boost::make_intrusive_ptr<ValueToValueMap>(map, from, to);
}

// () as ValueToValueMapBuilder
void systemCallValueToValueMapBuilderNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_intrusive_ptr<ValueToValueMapBuilder>();
}

// (builder as ValueToValueMapBuilder, key as Value, value as Value)
void systemCallValueToValueMapBuilderAdd(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto& builder = *castValueToValueMapBuilder(input.getObject(-1));
    const auto& from = input.getValue(-2);
    const auto& to = input.getValue(-1);
    builder.pairs.set(from, to);
}

// (builder as ValueToValueMapBuilder) as ValueToValueMap
void systemCallValueToValueMapBuilderEnd(const SystemCallInput& input, SystemCallResult* result) {
    auto& builder = *castValueToValueMapBuilder(input.getObject(-1));
    result->returnedObject = boost::make_intrusive_ptr<ValueToValueMap>(&builder);
}

// (input as Map from K to V, key as K) as Boolean
void systemCallMapContainsKey(const SystemCallInput& input, SystemCallResult* result) {
    // This could be any of the following signatures and we have to detect which it is:
    // 2 objects - ContainsKey(ObjectToObjectMap map, Object key) as Boolean
    // 2 objects - ContainsKey(ObjectToValueMap map, Object key) as Boolean
    // 1 object, 1 value - ContainsKey(ValueToObjectMap map, Value key) as Boolean
    // 1 object, 1 value - ContainsKey(ValueToValueMap map, Value key) as Boolean

    const auto isValueKey = input.numValueArguments > 0;
    const auto& mapObject = *input.getObject(isValueKey ? -1 : -2);

    switch (mapObject.getObjectType()) {
        case ObjectType::kObjectToObjectMap: {
            const auto& key = input.getObjectPtr(-1);
            const auto& map = castObjectToObjectMap(mapObject);
            const auto* found = map.pairs.find(key);
            result->returnedValue = Value{ found != nullptr };
            break;
        }

        case ObjectType::kObjectToValueMap: {
            const auto& key = input.getObjectPtr(-1);
            const auto& map = castObjectToValueMap(mapObject);
            const auto* found = map.pairs.find(key);
            result->returnedValue = Value{ found != nullptr };
            break;
        }

        case ObjectType::kValueToObjectMap: {
            const auto& key = input.getValue(-1);
            const auto& map = castValueToObjectMap(mapObject);
            const auto* found = map.pairs.find(key);
            result->returnedValue = Value{ found != nullptr };
            break;
        }

        case ObjectType::kValueToValueMap: {
            const auto& key = input.getValue(-1);
            const auto& map = castValueToValueMap(mapObject);
            const auto* found = map.pairs.find(key);
            result->returnedValue = Value{ found != nullptr };
            break;
        }

        default:
            throw Error(ErrorCode::kInternalTypeConfusion, "ContainsKey: first parameter isn't a map.");
    }
}

// (input as Map from K to V, key as K) as Optional V
void systemCallMapFind(const SystemCallInput& input, SystemCallResult* result) {
    // This could be any of the following signatures and we have to detect which it is:
    // 2 objects - Find(ObjectToObjectMap map, Object key) as Optional Object
    // 2 objects - Find(ObjectToValueMap map, Object key) as Optional Value
    // 1 object, 1 value - Find(ValueToObjectMap map, Value key) as Optional Object
    // 1 object, 1 value - Find(ValueToValueMap map, Value key) as Optional Value

    const auto isValueKey = input.numValueArguments > 0;
    const auto& mapObject = *input.getObject(isValueKey ? -1 : -2);

    switch (mapObject.getObjectType()) {
        case ObjectType::kObjectToObjectMap: {
            const auto& key = input.getObjectPtr(-1);
            const auto& map = castObjectToObjectMap(mapObject);
            const auto* found = map.pairs.find(key);
            if (found == nullptr) {
                result->returnedObject = boost::make_intrusive_ptr<ObjectOptional>();
            } else {
                result->returnedObject = boost::make_intrusive_ptr<ObjectOptional>(*found);
            }
            break;
        }

        case ObjectType::kObjectToValueMap: {
            const auto& key = input.getObjectPtr(-1);
            const auto& map = castObjectToValueMap(mapObject);
            const auto* found = map.pairs.find(key);
            if (found == nullptr) {
                result->returnedObject = boost::make_intrusive_ptr<ValueOptional>();
            } else {
                result->returnedObject = boost::make_intrusive_ptr<ValueOptional>(*found);
            }
            break;
        }

        case ObjectType::kValueToObjectMap: {
            const auto& key = input.getValue(-1);
            const auto& map = castValueToObjectMap(mapObject);
            const auto* found = map.pairs.find(key);
            if (found == nullptr) {
                result->returnedObject = boost::make_intrusive_ptr<ObjectOptional>();
            } else {
                result->returnedObject = boost::make_intrusive_ptr<ObjectOptional>(*found);
            }
            break;
        }

        case ObjectType::kValueToValueMap: {
            const auto& key = input.getValue(-1);
            const auto& map = castValueToValueMap(mapObject);
            const auto* found = map.pairs.find(key);
            if (found == nullptr) {
                result->returnedObject = boost::make_intrusive_ptr<ValueOptional>();
            } else {
                result->returnedObject = boost::make_intrusive_ptr<ValueOptional>(*found);
            }
            break;
        }

        default:
            throw Error(ErrorCode::kInternalTypeConfusion, "Find: first parameter isn't a map.");
    }
}

// (input as Map from K to V) as Number
void systemCallMapLen(const SystemCallInput& input, SystemCallResult* result) {
    const auto& mapObject = *input.getObject(-1);

    switch (mapObject.getObjectType()) {
        case ObjectType::kObjectToObjectMap: {
            const auto& map = castObjectToObjectMap(mapObject);
            result->returnedValue = Value{ static_cast<uint64_t>(map.pairs.size()) };
            break;
        }

        case ObjectType::kObjectToValueMap: {
            const auto& map = castObjectToValueMap(mapObject);
            result->returnedValue = Value{ static_cast<uint64_t>(map.pairs.size()) };
            break;
        }

        case ObjectType::kValueToObjectMap: {
            const auto& map = castValueToObjectMap(mapObject);
            result->returnedValue = Value{ static_cast<uint64_t>(map.pairs.size()) };
            break;
        }

        case ObjectType::kValueToValueMap: {
            const auto& map = castValueToValueMap(mapObject);
            result->returnedValue = Value{ static_cast<uint64_t>(map.pairs.size()) };
            break;
        }

        default:
            throw Error(ErrorCode::kInternalTypeConfusion, "Len: parameter isn't a map.");
    }
}

// (input as Map from K to V) as List of K
void systemCallMapObjectKeys(const SystemCallInput& input, SystemCallResult* result) {
    const auto& mapObject = *input.getObject(-1);

    switch (mapObject.getObjectType()) {
        case ObjectType::kObjectToObjectMap: {
            const auto& map = castObjectToObjectMap(mapObject);
            result->returnedObject = map.keys();
            break;
        }

        case ObjectType::kObjectToValueMap: {
            const auto& map = castObjectToValueMap(mapObject);
            result->returnedObject = map.keys();
            break;
        }

        default:
            throw Error(ErrorCode::kInternalTypeConfusion, "Keys: parameter isn't a map.");
    }
}

// (input as Map from K to V) as List of K
void systemCallMapValueKeys(const SystemCallInput& input, SystemCallResult* result) {
    const auto& mapObject = *input.getObject(-1);

    switch (mapObject.getObjectType()) {
        case ObjectType::kValueToObjectMap: {
            const auto& map = castValueToObjectMap(mapObject);
            result->returnedObject = map.keys();
            break;
        }

        case ObjectType::kValueToValueMap: {
            const auto& map = castValueToValueMap(mapObject);
            result->returnedObject = map.keys();
            break;
        }

        default:
            throw Error(ErrorCode::kInternalTypeConfusion, "Keys: parameter isn't a map.");
    }
}

// (input as Map from K to V) as List of V
void systemCallMapObjectValues(const SystemCallInput& input, SystemCallResult* result) {
    const auto& mapObject = *input.getObject(-1);

    switch (mapObject.getObjectType()) {
        case ObjectType::kObjectToObjectMap: {
            const auto& map = castObjectToObjectMap(mapObject);
            result->returnedObject = map.values();
            break;
        }

        case ObjectType::kValueToObjectMap: {
            const auto& map = castValueToObjectMap(mapObject);
            result->returnedObject = map.values();
            break;
        }

        default:
            throw Error(ErrorCode::kInternalTypeConfusion, "Values: parameter isn't a map.");
    }
}

// (input as Map from K to V) as List of V
void systemCallMapValueValues(const SystemCallInput& input, SystemCallResult* result) {
    const auto& mapObject = *input.getObject(-1);

    switch (mapObject.getObjectType()) {
        case ObjectType::kObjectToValueMap: {
            const auto& map = castObjectToValueMap(mapObject);
            result->returnedObject = map.values();
            break;
        }

        case ObjectType::kValueToValueMap: {
            const auto& map = castValueToValueMap(mapObject);
            result->returnedObject = map.values();
            break;
        }

        default:
            throw Error(ErrorCode::kInternalTypeConfusion, "Values: parameter isn't a map.");
    }
}

// (lhs as ValueToValueMap, rhs as ValueToValueMap) as ValueToValueMap
void systemCallValueToValueMapUnion(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map1 = castValueToValueMap(input.getObject(-2));
    const auto& map2 = castValueToValueMap(input.getObject(-1));
    result->returnedObject = map1->unionWith(*map2);
}

// (lhs as ValueToValueMap, rhs as ValueToValueMap) as ValueToValueMap
void systemCallValueToValueMapExcept(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map1 = castValueToValueMap(input.getObject(-2));
    const auto& map2 = castValueToValueMap(input.getObject(-1));
    result->returnedObject = map1->except(*map2);
}

// (lhs as ValueToObjectMap, rhs as ValueToObjectMap) as ValueToObjectMap
void systemCallValueToObjectMapUnion(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map1 = castValueToObjectMap(input.getObject(-2));
    const auto& map2 = castValueToObjectMap(input.getObject(-1));
    result->returnedObject = map1->unionWith(*map2);
}

// (lhs as ValueToObjectMap, rhs as ValueToObjectMap) as ValueToObjectMap
void systemCallValueToObjectMapExcept(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map1 = castValueToObjectMap(input.getObject(-2));
    const auto& map2 = castValueToObjectMap(input.getObject(-1));
    result->returnedObject = map1->except(*map2);
}

// (lhs as ObjectToValueMap, rhs as ObjectToValueMap) as ObjectToValueMap
void systemCallObjectToValueMapUnion(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map1 = castObjectToValueMap(input.getObject(-2));
    const auto& map2 = castObjectToValueMap(input.getObject(-1));
    result->returnedObject = map1->unionWith(*map2);
}

// (lhs as ObjectToValueMap, rhs as ObjectToValueMap) as ObjectToValueMap
void systemCallObjectToValueMapExcept(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map1 = castObjectToValueMap(input.getObject(-2));
    const auto& map2 = castObjectToValueMap(input.getObject(-1));
    result->returnedObject = map1->except(*map2);
}

// (lhs as ObjectToObjectMap, rhs as ObjectToObjectMap) as ObjectToObjectMap
void systemCallObjectToObjectMapUnion(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map1 = castObjectToObjectMap(input.getObject(-2));
    const auto& map2 = castObjectToObjectMap(input.getObject(-1));
    result->returnedObject = map1->unionWith(*map2);
}

// (lhs as ObjectToObjectMap, rhs as ObjectToObjectMap) as ObjectToObjectMap
void systemCallObjectToObjectMapExcept(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map1 = castObjectToObjectMap(input.getObject(-2));
    const auto& map2 = castObjectToObjectMap(input.getObject(-1));
    result->returnedObject = map1->except(*map2);
}

}  // namespace vm
