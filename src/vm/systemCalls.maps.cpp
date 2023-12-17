#include "vm/systemCall.h"
#include "vm/castObject.h"
#include "vm/Error.h"
#include "vm/Map.h"

namespace vm {

void initSystemCallsMaps() {
    initSystemCall(SystemCall::kObjectToObjectMapNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ObjectToObjectMap>();
    });

    initSystemCall(SystemCall::kObjectToObjectMapSet, [](const auto& input, auto* result) {
        const auto& map = castObjectToObjectMap(input.getObject(-3));
        const auto& from = input.getObjectPtr(-2);
        const auto& to = input.getObjectPtr(-1);
        result->returnedObject = boost::make_local_shared<ObjectToObjectMap>(map, from, to);
    });

    initSystemCall(SystemCall::kObjectToValueMapNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ObjectToValueMap>();
    });

    initSystemCall(SystemCall::kObjectToValueMapSet, [](const auto& input, auto* result) {
        const auto& map = castObjectToValueMap(input.getObject(-2));
        const auto& from = input.getObjectPtr(-1);
        const auto& to = input.getValue(-1);
        result->returnedObject = boost::make_local_shared<ObjectToValueMap>(map, from, to);
    });

    initSystemCall(SystemCall::kValueToObjectMapNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ValueToObjectMap>();
    });

    initSystemCall(SystemCall::kValueToObjectMapSet, [](const auto& input, auto* result) {
        const auto& map = castValueToObjectMap(input.getObject(-2));
        const auto& from = input.getValue(-1);
        const auto& to = input.getObjectPtr(-1);
        result->returnedObject = boost::make_local_shared<ValueToObjectMap>(map, from, to);
    });

    initSystemCall(SystemCall::kValueToValueMapNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ValueToValueMap>();
    });

    initSystemCall(SystemCall::kValueToValueMapSet, [](const auto& input, auto* result) {
        const auto& map = castValueToValueMap(input.getObject(-1));
        const auto& from = input.getValue(-2);
        const auto& to = input.getValue(-1);
        result->returnedObject = boost::make_local_shared<ValueToValueMap>(map, from, to);
    });

    initSystemCall(SystemCall::kMapContainsKey, [](const auto& input, auto* result) {
        // This could be any of the following signatures and we have to detect which it is:
        // 2 objects - ContainsKey(ObjectToObjectMap map, Object key) as Boolean
        // 2 objects - ContainsKey(ObjectToValueMap map, Object key) as Boolean
        // 1 object, 1 value - ContainsKey(ValueToObjectMap map, Value key) as Boolean
        // 1 object, 1 value - ContainsKey(ValueToValueMap map, Value key) as Boolean

        const auto isValueKey = input.numValueArguments > 0;
        const auto& mapObject = input.getObject(isValueKey ? -1 : -2);

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
    });

    initSystemCall(SystemCall::kMapFind, [](const auto& input, auto* result) {
        // This could be any of the following signatures and we have to detect which it is:
        // 2 objects - Find(ObjectToObjectMap map, Object key) as Optional Object
        // 2 objects - Find(ObjectToValueMap map, Object key) as Optional Value
        // 1 object, 1 value - Find(ValueToObjectMap map, Value key) as Optional Object
        // 1 object, 1 value - Find(ValueToValueMap map, Value key) as Optional Value

        const auto isValueKey = input.numValueArguments > 0;
        const auto& mapObject = input.getObject(isValueKey ? -1 : -2);

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
                throw Error(ErrorCode::kInternalTypeConfusion, "ContainsKey: first parameter isn't a map.");
        }
    });
}

}  // namespace vm
