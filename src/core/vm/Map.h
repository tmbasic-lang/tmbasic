#pragma once

#include "common.h"
#include "List.h"
#include "Object.h"
#include "Value.h"

namespace vm {

template <
    typename TKey,
    typename TValue,
    Kind K,
    typename TKeyListBuilder,
    typename TKeyList,
    typename TValuePointerCompare,
    typename TValueListBuilder,
    typename TValueList>
class Map : public Object {
   public:
    const immer::map<TKey, TValue> pairs;

    Map() {}

    Map(Map<TKey, TValue, K, TKeyListBuilder, TKeyList, TValuePointerCompare, TValueListBuilder, TValueList>& source,
        TKey newKey,
        TValue newValue)
        : pairs(std::move(source.pairs.set(newKey, newValue))) {}

    Map(Map<TKey, TValue, K, TKeyListBuilder, TKeyList, TValuePointerCompare, TValueListBuilder, TValueList>& source,
        TKey removeKey)
        : pairs(std::move(source.pairs.erase(removeKey))) {}

    Kind getKind() const override { return K; }

    size_t getHash() const override { return std::hash<size_t>{}(pairs.size()); }

    bool equals(const Object& other) const override {
        if (other.getKind() != K) {
            return false;
        }
        auto& otherMap = static_cast<const Map<
            TKey, TValue, K, TKeyListBuilder, TKeyList, TValuePointerCompare, TValueListBuilder, TValueList>&>(other);
        if (pairs.size() != otherMap.pairs.size()) {
            return false;
        }
        TValuePointerCompare equals;
        for (auto pair : pairs) {
            auto otherValuePtr = otherMap.pairs.find(pair.first);
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
        return boost::make_local_shared<TKeyList>(builder);
    }

    boost::local_shared_ptr<TValueList> values() const {
        TValueListBuilder builder;
        for (auto& pair : pairs) {
            builder.items.push_back(pair.second);
        }
        return boost::make_local_shared<TValueList>(builder);
    }
};

typedef Map<
    Value,
    Value,
    Kind::kValueToValueMap,
    ValueListBuilder,
    ValueList,
    ValuePointerCompare,
    ValueListBuilder,
    ValueList>
    ValueToValueMap;

typedef Map<
    Value,
    boost::local_shared_ptr<Object>,
    Kind::kValueToObjectMap,
    ValueListBuilder,
    ValueList,
    ObjectPointerCompare,
    ObjectListBuilder,
    ObjectList>
    ValueToObjectMap;

typedef Map<
    boost::local_shared_ptr<Object>,
    Value,
    Kind::kObjectToValueMap,
    ObjectListBuilder,
    ObjectList,
    ValuePointerCompare,
    ValueListBuilder,
    ValueList>
    ObjectToValueMap;

typedef Map<
    boost::local_shared_ptr<Object>,
    boost::local_shared_ptr<Object>,
    Kind::kObjectToObjectMap,
    ObjectListBuilder,
    ObjectList,
    ObjectPointerCompare,
    ObjectListBuilder,
    ObjectList>
    ObjectToObjectMap;

}  // namespace vm
