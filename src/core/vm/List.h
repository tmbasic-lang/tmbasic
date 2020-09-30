#pragma once

#include "common.h"
#include "Kind.h"
#include "Object.h"
#include "Value.h"

namespace vm {

template <typename TElement>
class ListBuilder {
   public:
    immer::vector_transient<TElement> items;
};

typedef ListBuilder<boost::local_shared_ptr<Object>> ObjectListBuilder;
typedef ListBuilder<Value> ValueListBuilder;

template <typename TElement, Kind K>
class List : public Object {
   public:
    const immer::vector<TElement> items;

    List(ListBuilder<TElement>& builder) : items(std::move(builder.items.persistent())) {}

    List(List<TElement, K>& source, int removeIndex) : items(std::move(removeAt(source, removeIndex))) {}

    List(List<TElement, K>& source, bool insert, int index, TElement newElement)
        : items(std::move(insertOrSetAt(source, insert, index, newElement))) {}

    Kind getKind() const override { return K; }

    size_t getHash() const override {
        size_t hash = 17;
        auto count = items.size();
        for (size_t i = 0; i < 5 && i < count; i++) {
            hash = hash * 23 + std::hash<TElement>{}(items[i]);
        }
        return hash;
    }

    bool equals(const Object& other) const override {
        if (other.getKind() != K) {
            return false;
        }
        auto& otherList = static_cast<const List<TElement, K>&>(other);
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

   private:
    static immer::vector<TElement> removeAt(const immer::vector<TElement>& source, int removeIndex) {
        auto t = source.take(removeIndex).transient();
        for (int i = removeIndex + 1; i < source.size(); i++) {
            t.push_back(source[i]);
        }
        return t.persistent();
    }

    static immer::vector<TElement> insertOrSetAt(
        const immer::vector<TElement>& source,
        bool insert,
        int index,
        TElement newElement) {
        if (insert && index == source.size()) {
            return source.push_back(newElement);
        }
        auto t = source.take(index).transient();
        t.push_back(newElement);
        for (int i = insert ? index : index + 1; i < source.size(); i++) {
            t.push_back(source[i]);
        }
        return t.persistent();
    }
};

typedef List<boost::local_shared_ptr<Object>, Kind::kObjectList> ObjectList;
typedef List<Value, Kind::kValueList> ValueList;

}  // namespace vm
