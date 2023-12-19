#pragma once

#include "../common.h"
#include "vm/List.h"
#include "vm/Object.h"
#include "vm/Value.h"

namespace vm {

template <typename TKey, ObjectType K, typename TKeyHash, typename TKeyEqual>
class SetBuilder : public Object {
   public:
    immer::set_transient<TKey, TKeyHash, TKeyEqual> keys;
    SetBuilder() {}
    explicit SetBuilder(immer::set_transient<TKey> keys) : keys(std::move(keys)) {}
    ObjectType getObjectType() const override { return K; }
    std::size_t getHash() const override { return 0; }
    bool equals(const Object& other) const override { return false; }
};

using ValueSetBuilder = SetBuilder<Value, ObjectType::kValueSetBuilder, std::hash<Value>, std::equal_to<Value>>;

using ObjectSetBuilder = SetBuilder<
    boost::local_shared_ptr<Object>,
    ObjectType::kObjectSetBuilder,
    std::hash<boost::local_shared_ptr<Object>>,
    ObjectReferenceCompare>;

template <
    typename TKey,
    ObjectType K,
    typename TKeyListBuilder,
    typename TKeyList,
    typename TKeyHash,
    typename TKeyEqual,
    typename TSetBuilder>
class Set : public Object {
   public:
    using Self = Set<TKey, K, TKeyListBuilder, TKeyList, TKeyHash, TKeyEqual, TSetBuilder>;

    const immer::set<TKey, TKeyHash, TKeyEqual> keys = {};

    Set() = default;

    explicit Set(TSetBuilder* builder) : keys(builder->keys.persistent()) {}

    static immer::set<TKey, TKeyHash, TKeyEqual> unionSets(const Self& a, const Self& b) {
        TSetBuilder builder;
        for (const auto& key : a.keys) {
            builder.keys.insert(key);
        }
        for (const auto& key : b.keys) {
            builder.keys.insert(key);
        }
        return builder.keys.persistent();
    }

    Set(const Self& a, const Self& b) : keys(unionSets(a, b)) {}

    Set(const Self& source, bool insert, TKey key) : keys(insert ? source.keys.insert(key) : source.keys.erase(key)) {}

    ObjectType getObjectType() const override { return K; }

    std::size_t getHash() const override { return std::hash<size_t>{}(keys.size()); }

    bool equals(const Object& other) const override {
        if (other.getObjectType() != K) {
            return false;
        }
        const auto& otherSet = static_cast<const Self&>(other);
        if (keys.size() != otherSet.keys.size()) {
            return false;
        }
        for (const auto& key : keys) {
            if (otherSet.keys.find(key) == nullptr) {
                return false;
            }
        }
        return true;
    }

    boost::local_shared_ptr<TKeyList> toList() const {
        TKeyListBuilder builder;
        for (const auto& key : keys) {
            builder.items.push_back(key);
        }
        return boost::make_local_shared<TKeyList>(&builder);
    }
};

using ValueSet =
    Set<Value,
        ObjectType::kValueSet,
        ValueListBuilder,
        ValueList,
        std::hash<Value>,
        std::equal_to<Value>,
        ValueSetBuilder>;

using ObjectSet =
    Set<boost::local_shared_ptr<Object>,
        ObjectType::kObjectSet,
        ObjectListBuilder,
        ObjectList,
        std::hash<boost::local_shared_ptr<Object>>,
        ObjectReferenceCompare,
        ObjectSetBuilder>;

}  // namespace vm
