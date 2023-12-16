#pragma once

#include "../common.h"
#include "vm/Object.h"
#include "vm/Value.h"

namespace vm {

template <typename TElement, ObjectType K>
class ListBuilder : public Object {
   public:
    immer::vector_transient<TElement> items;
    ListBuilder() {}
    explicit ListBuilder(immer::vector_transient<TElement> items) : items(std::move(items)) {}
    ObjectType getObjectType() const override { return K; }
    std::size_t getHash() const override { return 0; }
    bool equals(const Object& other) const override { return false; }
};

using ObjectListBuilder = ListBuilder<boost::local_shared_ptr<Object>, ObjectType::kObjectListBuilder>;
using ValueListBuilder = ListBuilder<Value, ObjectType::kValueListBuilder>;

class ListBase : public Object {
   public:
    virtual size_t size() const = 0;
};

template <typename TElement, ObjectType K, typename TListBuilder>
class List : public ListBase {
   public:
    const immer::vector<TElement> items = {};

    explicit List(TListBuilder* builder) : items(std::move(builder->items.persistent())) {}

    List(const List<TElement, K, TListBuilder>& source, size_t removeIndex)
        : items(std::move(removeAt(source.items, removeIndex))) {}

    List(const List<TElement, K, TListBuilder>& source, bool insert, size_t index, TElement newElement)
        : items(std::move(insertOrSetAt(source.items, insert, index, newElement))) {}

    ObjectType getObjectType() const override { return K; }

    size_t getHash() const override {
        size_t hash = 17;
        auto count = items.size();
        for (size_t i = 0; i < 5 && i < count; i++) {
            hash = hash * 23 + std::hash<TElement>{}(items[i]);
        }
        return hash;
    }

    bool equals(const Object& other) const override {
        if (other.getObjectType() != K) {
            return false;
        }
        auto& otherList = dynamic_cast<const List<TElement, K, TListBuilder>&>(other);
        if (items.size() != otherList.items.size()) {
            return false;
        }
        auto count = items.size();
        std::equal_to<TElement> equals;
        for (size_t i = 0; i < count; i++) {
            if (!equals(items[i], otherList.items[i])) {
                return false;
            }
        }
        return true;
    }

    size_t size() const override { return items.size(); }

   private:
    static immer::vector<TElement> removeAt(const immer::vector<TElement>& source, size_t removeIndex) {
        auto t = source.take(removeIndex).transient();
        for (size_t i = removeIndex + 1; i < source.size(); i++) {
            t.push_back(source[i]);
        }
        return t.persistent();
    }

    static immer::vector<TElement> insertOrSetAt(
        const immer::vector<TElement>& source,
        bool insert,
        size_t index,
        TElement newElement) {
        if (insert && index == source.size()) {
            return source.push_back(newElement);
        }
        auto t = source.take(index).transient();
        t.push_back(newElement);
        for (size_t i = insert ? index : index + 1; i < source.size(); i++) {
            t.push_back(source[i]);
        }
        return t.persistent();
    }
};

using ObjectList = List<boost::local_shared_ptr<Object>, ObjectType::kObjectList, ObjectListBuilder>;
using ValueList = List<Value, ObjectType::kValueList, ValueListBuilder>;

}  // namespace vm
