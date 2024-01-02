#include "vm/systemCall.h"
#include "vm/castObject.h"
#include "vm/Error.h"
#include "vm/Map.h"

namespace vm {

void systemCallObjectToObjectMapNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_local_shared<ObjectToObjectMap>();
}

void systemCallObjectToObjectMapSet(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map = *castObjectToObjectMap(input.getObject(-3));
    const auto& from = input.getObjectPtr(-2);
    const auto& to = input.getObjectPtr(-1);
    result->returnedObject = boost::make_local_shared<ObjectToObjectMap>(map, from, to);
}

void systemCallObjectToObjectMapBuilderNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_local_shared<ObjectToObjectMapBuilder>();
}

void systemCallObjectToObjectMapBuilderAdd(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto& builder = *castObjectToObjectMapBuilder(input.getObject(-3));
    const auto& from = input.getObjectPtr(-2);
    const auto& to = input.getObjectPtr(-1);
    builder.pairs.set(from, to);
}

void systemCallObjectToObjectMapBuilderEnd(const SystemCallInput& input, SystemCallResult* result) {
    auto& builder = *castObjectToObjectMapBuilder(input.getObject(-1));
    result->returnedObject = boost::make_local_shared<ObjectToObjectMap>(&builder);
}

void systemCallObjectToValueMapNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_local_shared<ObjectToValueMap>();
}

void systemCallObjectToValueMapSet(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map = *castObjectToValueMap(input.getObject(-2));
    const auto& from = input.getObjectPtr(-1);
    const auto& to = input.getValue(-1);
    result->returnedObject = boost::make_local_shared<ObjectToValueMap>(map, from, to);
}

void systemCallObjectToValueMapBuilderNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_local_shared<ObjectToValueMapBuilder>();
}

void systemCallObjectToValueMapBuilderAdd(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto& builder = *castObjectToValueMapBuilder(input.getObject(-2));
    const auto& from = input.getObjectPtr(-1);
    const auto& to = input.getValue(-1);
    builder.pairs.set(from, to);
}

void systemCallObjectToValueMapBuilderEnd(const SystemCallInput& input, SystemCallResult* result) {
    auto& builder = *castObjectToValueMapBuilder(input.getObject(-1));
    result->returnedObject = boost::make_local_shared<ObjectToValueMap>(&builder);
}

void systemCallValueToObjectMapNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_local_shared<ValueToObjectMap>();
}

void systemCallValueToObjectMapSet(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map = *castValueToObjectMap(input.getObject(-2));
    const auto& from = input.getValue(-1);
    const auto& to = input.getObjectPtr(-1);
    result->returnedObject = boost::make_local_shared<ValueToObjectMap>(map, from, to);
}

void systemCallValueToObjectMapBuilderNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_local_shared<ValueToObjectMapBuilder>();
}

void systemCallValueToObjectMapBuilderAdd(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto& builder = *castValueToObjectMapBuilder(input.getObject(-2));
    const auto& from = input.getValue(-1);
    const auto& to = input.getObjectPtr(-1);
    builder.pairs.set(from, to);
}

void systemCallValueToObjectMapBuilderEnd(const SystemCallInput& input, SystemCallResult* result) {
    auto& builder = *castValueToObjectMapBuilder(input.getObject(-1));
    result->returnedObject = boost::make_local_shared<ValueToObjectMap>(&builder);
}

void systemCallValueToValueMapNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_local_shared<ValueToValueMap>();
}

void systemCallValueToValueMapSet(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map = *castValueToValueMap(input.getObject(-1));
    const auto& from = input.getValue(-2);
    const auto& to = input.getValue(-1);
    result->returnedObject = boost::make_local_shared<ValueToValueMap>(map, from, to);
}

void systemCallValueToValueMapBuilderNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_local_shared<ValueToValueMapBuilder>();
}

void systemCallValueToValueMapBuilderAdd(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto& builder = *castValueToValueMapBuilder(input.getObject(-1));
    const auto& from = input.getValue(-2);
    const auto& to = input.getValue(-1);
    builder.pairs.set(from, to);
}

void systemCallValueToValueMapBuilderEnd(const SystemCallInput& input, SystemCallResult* result) {
    auto& builder = *castValueToValueMapBuilder(input.getObject(-1));
    result->returnedObject = boost::make_local_shared<ValueToValueMap>(&builder);
}

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
                result->returnedObject = boost::make_local_shared<ObjectOptional>();
            } else {
                result->returnedObject = boost::make_local_shared<ObjectOptional>(*found);
            }
            break;
        }

        case ObjectType::kObjectToValueMap: {
            const auto& key = input.getObjectPtr(-1);
            const auto& map = castObjectToValueMap(mapObject);
            const auto* found = map.pairs.find(key);
            if (found == nullptr) {
                result->returnedObject = boost::make_local_shared<ValueOptional>();
            } else {
                result->returnedObject = boost::make_local_shared<ValueOptional>(*found);
            }
            break;
        }

        case ObjectType::kValueToObjectMap: {
            const auto& key = input.getValue(-1);
            const auto& map = castValueToObjectMap(mapObject);
            const auto* found = map.pairs.find(key);
            if (found == nullptr) {
                result->returnedObject = boost::make_local_shared<ObjectOptional>();
            } else {
                result->returnedObject = boost::make_local_shared<ObjectOptional>(*found);
            }
            break;
        }

        case ObjectType::kValueToValueMap: {
            const auto& key = input.getValue(-1);
            const auto& map = castValueToValueMap(mapObject);
            const auto* found = map.pairs.find(key);
            if (found == nullptr) {
                result->returnedObject = boost::make_local_shared<ValueOptional>();
            } else {
                result->returnedObject = boost::make_local_shared<ValueOptional>(*found);
            }
            break;
        }

        default:
            throw Error(ErrorCode::kInternalTypeConfusion, "Find: first parameter isn't a map.");
    }
}

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

void systemCallValueToValueMapUnion(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map1 = castValueToValueMap(input.getObject(-2));
    const auto& map2 = castValueToValueMap(input.getObject(-1));
    result->returnedObject = map1->unionWith(*map2);
}

void systemCallValueToValueMapExcept(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map1 = castValueToValueMap(input.getObject(-2));
    const auto& map2 = castValueToValueMap(input.getObject(-1));
    result->returnedObject = map1->except(*map2);
}

void systemCallValueToObjectMapUnion(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map1 = castValueToObjectMap(input.getObject(-2));
    const auto& map2 = castValueToObjectMap(input.getObject(-1));
    result->returnedObject = map1->unionWith(*map2);
}

void systemCallValueToObjectMapExcept(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map1 = castValueToObjectMap(input.getObject(-2));
    const auto& map2 = castValueToObjectMap(input.getObject(-1));
    result->returnedObject = map1->except(*map2);
}

void systemCallObjectToValueMapUnion(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map1 = castObjectToValueMap(input.getObject(-2));
    const auto& map2 = castObjectToValueMap(input.getObject(-1));
    result->returnedObject = map1->unionWith(*map2);
}

void systemCallObjectToValueMapExcept(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map1 = castObjectToValueMap(input.getObject(-2));
    const auto& map2 = castObjectToValueMap(input.getObject(-1));
    result->returnedObject = map1->except(*map2);
}

void systemCallObjectToObjectMapUnion(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map1 = castObjectToObjectMap(input.getObject(-2));
    const auto& map2 = castObjectToObjectMap(input.getObject(-1));
    result->returnedObject = map1->unionWith(*map2);
}

void systemCallObjectToObjectMapExcept(const SystemCallInput& input, SystemCallResult* result) {
    const auto& map1 = castObjectToObjectMap(input.getObject(-2));
    const auto& map2 = castObjectToObjectMap(input.getObject(-1));
    result->returnedObject = map1->except(*map2);
}

}  // namespace vm
