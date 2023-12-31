#pragma once

#include "../common.h"
#include "vm/List.h"
#include "vm/Object.h"
#include "vm/Value.h"
#include "vm/RecordBuilder.h"
#include "vm/Record.h"

namespace vm {

template <typename TKey, typename TValue, ObjectType K, typename TKeyHash, typename TKeyEqual>
class MapBuilder : public Object {
   public:
    immer::map_transient<TKey, TValue, TKeyHash, TKeyEqual> pairs;
    MapBuilder() {}
    explicit MapBuilder(immer::map_transient<TKey, TValue> pairs) : pairs(std::move(pairs)) {}
    ObjectType getObjectType() const override { return K; }
    std::size_t getHash() const override { return 0; }
    bool equals(const Object& other) const override { return false; }
};

using ValueToValueMapBuilder =
    MapBuilder<Value, Value, ObjectType::kValueToValueMapBuilder, std::hash<Value>, std::equal_to<Value>>;

using ValueToObjectMapBuilder = MapBuilder<
    Value,
    boost::local_shared_ptr<Object>,
    ObjectType::kValueToObjectMapBuilder,
    std::hash<Value>,
    std::equal_to<Value>>;

using ObjectToValueMapBuilder = MapBuilder<
    boost::local_shared_ptr<Object>,
    Value,
    ObjectType::kObjectToValueMapBuilder,
    std::hash<boost::local_shared_ptr<Object>>,
    ObjectReferenceCompare>;

using ObjectToObjectMapBuilder = MapBuilder<
    boost::local_shared_ptr<Object>,
    boost::local_shared_ptr<Object>,
    ObjectType::kObjectToObjectMapBuilder,
    std::hash<boost::local_shared_ptr<Object>>,
    ObjectReferenceCompare>;

template <
    typename TKey,
    typename TValue,
    ObjectType K,
    typename TKeyListBuilder,
    typename TKeyList,
    typename TKeyHash,
    typename TKeyEqual,
    typename TValuePointerCompare,
    typename TValueListBuilder,
    typename TValueList,
    typename TMapBuilder>
class Map : public Object {
   public:
    using Self =
        Map<TKey,
            TValue,
            K,
            TKeyListBuilder,
            TKeyList,
            TKeyHash,
            TKeyEqual,
            TValuePointerCompare,
            TValueListBuilder,
            TValueList,
            TMapBuilder>;

    const immer::map<TKey, TValue, TKeyHash, TKeyEqual> pairs = {};

    Map() = default;

    explicit Map(TMapBuilder* builder) : pairs(builder->pairs.persistent()) {}

    Map(const Self& source, TKey newKey, TValue newValue) : pairs(source.pairs.set(newKey, newValue)) {}

    Map(const Self& source, TKey removeKey) : pairs(source.pairs.erase(removeKey)) {}

    ObjectType getObjectType() const override { return K; }

    size_t getHash() const override { return std::hash<size_t>{}(pairs.size()); }

    bool equals(const Object& other) const override {
        if (other.getObjectType() != K) {
            return false;
        }
        auto& otherMap = static_cast<const Self&>(other);
        if (pairs.size() != otherMap.pairs.size()) {
            return false;
        }
        TValuePointerCompare equals;
        for (const auto& pair : pairs) {
            const auto* otherValuePtr = otherMap.pairs.find(pair.first);
            if (otherValuePtr == nullptr) {
                return false;
            }
            if (!equals(pair.second, otherValuePtr)) {
                return false;
            }
        }
        return true;
    }

    boost::local_shared_ptr<TKeyList> keys() const {
        TKeyListBuilder builder;
        for (auto& pair : pairs) {
            builder.items.push_back(pair.first);
        }
        return boost::make_local_shared<TKeyList>(&builder);
    }

    boost::local_shared_ptr<TValueList> values() const {
        TValueListBuilder builder;
        for (auto& pair : pairs) {
            builder.items.push_back(pair.second);
        }
        return boost::make_local_shared<TValueList>(&builder);
    }

    boost::local_shared_ptr<Record> keysAndValues() const {
        TKeyListBuilder keysBuilder;
        TValueListBuilder valuesBuilder;
        for (auto& pair : pairs) {
            keysBuilder.items.push_back(pair.first);
            valuesBuilder.items.push_back(pair.second);
        }

        RecordBuilder builder{ 0, 2 };
        builder.objects.push_back(boost::make_local_shared<TKeyList>(&keysBuilder));
        builder.objects.push_back(boost::make_local_shared<TValueList>(&valuesBuilder));
        return boost::make_local_shared<Record>(&builder);
    }
};

using ValueToValueMap =
    Map<Value,
        Value,
        ObjectType::kValueToValueMap,
        ValueListBuilder,
        ValueList,
        std::hash<Value>,
        std::equal_to<Value>,
        ValuePointerCompare,
        ValueListBuilder,
        ValueList,
        ValueToValueMapBuilder>;

using ValueToObjectMap =
    Map<Value,
        boost::local_shared_ptr<Object>,
        ObjectType::kValueToObjectMap,
        ValueListBuilder,
        ValueList,
        std::hash<Value>,
        std::equal_to<Value>,
        ObjectPointerCompare,
        ObjectListBuilder,
        ObjectList,
        ValueToObjectMapBuilder>;

using ObjectToValueMap =
    Map<boost::local_shared_ptr<Object>,
        Value,
        ObjectType::kObjectToValueMap,
        ObjectListBuilder,
        ObjectList,
        std::hash<boost::local_shared_ptr<Object>>,
        ObjectReferenceCompare,
        ValuePointerCompare,
        ValueListBuilder,
        ValueList,
        ObjectToValueMapBuilder>;

using ObjectToObjectMap =
    Map<boost::local_shared_ptr<Object>,
        boost::local_shared_ptr<Object>,
        ObjectType::kObjectToObjectMap,
        ObjectListBuilder,
        ObjectList,
        std::hash<boost::local_shared_ptr<Object>>,
        ObjectReferenceCompare,
        ObjectPointerCompare,
        ObjectListBuilder,
        ObjectList,
        ObjectToObjectMapBuilder>;

}  // namespace vm
